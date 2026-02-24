#include "interpreter/natives/native_io.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif
#ifdef CLAW_HAS_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#endif

namespace claw {

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

#ifdef _WIN32
static std::vector<uint8_t> randomBytes(size_t n) {
    std::vector<uint8_t> out(n);
    BCryptGenRandom(nullptr, out.data(), (ULONG)out.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return out;
}

static std::vector<uint8_t> deriveKeyPBKDF2(const std::string& pass, const std::vector<uint8_t>& salt, uint32_t iterations, size_t keyLen) {
    BCRYPT_ALG_HANDLE hHash = nullptr;
    if (BCryptOpenAlgorithmProvider(&hHash, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) {
        throw std::runtime_error("BCrypt: SHA256 provider open failed");
    }
    std::vector<uint8_t> key(keyLen);
    NTSTATUS st = BCryptDeriveKeyPBKDF2(hHash,
        reinterpret_cast<UCHAR*>(const_cast<char*>(pass.data())), (ULONG)pass.size(),
        const_cast<UCHAR*>(salt.data()), (ULONG)salt.size(),
        iterations, key.data(), (ULONG)key.size(), 0);
    BCryptCloseAlgorithmProvider(hHash, 0);
    if (st != 0) {
        // Fallback: simple iterative SHA256-based KDF
        BCRYPT_ALG_HANDLE hSha = nullptr;
        if (BCryptOpenAlgorithmProvider(&hSha, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) {
            throw std::runtime_error("BCrypt: PBKDF2 derive failed");
        }
        std::vector<uint8_t> buf(salt.size() + pass.size() + 4);
        memcpy(buf.data(), salt.data(), salt.size());
        memcpy(buf.data() + salt.size(), pass.data(), pass.size());
        for (uint32_t i = 0; i < iterations; ++i) {
            uint32_t ctr = _byteswap_ulong(i + 1);
            memcpy(buf.data() + salt.size() + pass.size(), &ctr, 4);
            BCRYPT_HASH_HANDLE h = nullptr;
            if (BCryptCreateHash(hSha, &h, nullptr, 0, nullptr, 0, 0) != 0) {
                BCryptCloseAlgorithmProvider(hSha, 0);
                throw std::runtime_error("BCrypt: PBKDF2 derive failed");
            }
            DWORD outLen = 0;
            if (BCryptHashData(h, buf.data(), (ULONG)buf.size(), 0) != 0 ||
                BCryptFinishHash(h, key.data(), (ULONG)key.size(), 0) != 0) {
                BCryptDestroyHash(h);
                BCryptCloseAlgorithmProvider(hSha, 0);
                throw std::runtime_error("BCrypt: PBKDF2 derive failed");
            }
            BCryptDestroyHash(h);
        }
        BCryptCloseAlgorithmProvider(hSha, 0);
    }
    return key;
}

static std::vector<uint8_t> aesGcmEncrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce,
                                          const std::vector<uint8_t>& aad, const std::vector<uint8_t>& plaintext,
                                          std::vector<uint8_t>& tagOut) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (st != 0) throw std::runtime_error("BCrypt: AES provider open failed");
    wchar_t modeName[] = BCRYPT_CHAIN_MODE_GCM;
    st = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, reinterpret_cast<UCHAR*>(modeName), sizeof(modeName), 0);
    if (st != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: set GCM mode failed"); }
    DWORD keyObjLen = 0, res = 0;
    st = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<UCHAR*>(&keyObjLen), sizeof(DWORD), &res, 0);
    if (st != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: get key obj len failed"); }
    std::vector<UCHAR> keyObj(keyObjLen);
    st = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj.data(), keyObjLen, const_cast<UCHAR*>(key.data()), (ULONG)key.size(), 0);
    if (st != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: generate key failed"); }
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;
    BCRYPT_INIT_AUTH_MODE_INFO(info);
    info.pbNonce = const_cast<UCHAR*>(nonce.data());
    info.cbNonce = (ULONG)nonce.size();
    info.pbAuthData = const_cast<UCHAR*>(aad.data());
    info.cbAuthData = (ULONG)aad.size();
    tagOut.resize(16);
    info.pbTag = tagOut.data();
    info.cbTag = (ULONG)tagOut.size();
    DWORD outLen = 0;
    st = BCryptEncrypt(hKey, const_cast<UCHAR*>(plaintext.data()), (ULONG)plaintext.size(), &info,
                       nullptr, 0, nullptr, 0, &outLen, 0);
    if (st != 0) { BCryptDestroyKey(hKey); BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: size query failed"); }
    std::vector<uint8_t> ciphertext(outLen);
    st = BCryptEncrypt(hKey, const_cast<UCHAR*>(plaintext.data()), (ULONG)plaintext.size(), &info,
                       nullptr, 0, ciphertext.data(), outLen, &outLen, 0);
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (st != 0) throw std::runtime_error("BCrypt: encrypt failed");
    return ciphertext;
}

static std::vector<uint8_t> aesGcmDecrypt(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce,
                                          const std::vector<uint8_t>& aad, const std::vector<uint8_t>& ciphertext,
                                          const std::vector<uint8_t>& tag) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (st != 0) throw std::runtime_error("BCrypt: AES provider open failed");
    wchar_t modeName[] = BCRYPT_CHAIN_MODE_GCM;
    st = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, reinterpret_cast<UCHAR*>(modeName), sizeof(modeName), 0);
    if (st != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: set GCM mode failed"); }
    DWORD keyObjLen = 0, res = 0;
    st = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<UCHAR*>(&keyObjLen), sizeof(DWORD), &res, 0);
    if (st != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: get key obj len failed"); }
    std::vector<UCHAR> keyObj(keyObjLen);
    st = BCryptGenerateSymmetricKey(hAlg, &hKey, keyObj.data(), keyObjLen, const_cast<UCHAR*>(key.data()), (ULONG)key.size(), 0);
    if (st != 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: generate key failed"); }
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;
    BCRYPT_INIT_AUTH_MODE_INFO(info);
    info.pbNonce = const_cast<UCHAR*>(nonce.data());
    info.cbNonce = (ULONG)nonce.size();
    info.pbAuthData = const_cast<UCHAR*>(aad.data());
    info.cbAuthData = (ULONG)aad.size();
    info.pbTag = const_cast<UCHAR*>(tag.data());
    info.cbTag = (ULONG)tag.size();
    DWORD outLen = 0;
    st = BCryptDecrypt(hKey, const_cast<UCHAR*>(ciphertext.data()), (ULONG)ciphertext.size(), &info,
                       nullptr, 0, nullptr, 0, &outLen, 0);
    if (st != 0) { BCryptDestroyKey(hKey); BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCrypt: size query failed"); }
    std::vector<uint8_t> plaintext(outLen);
    st = BCryptDecrypt(hKey, const_cast<UCHAR*>(ciphertext.data()), (ULONG)ciphertext.size(), &info,
                       nullptr, 0, plaintext.data(), outLen, &outLen, 0);
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (st != 0) throw std::runtime_error("BCrypt: decrypt failed (tag mismatch?)");
    return plaintext;
}
#endif
#ifdef CLAW_HAS_OPENSSL
static std::vector<uint8_t> randomBytesOpenSSL(size_t n) {
    std::vector<uint8_t> out(n);
    RAND_bytes(out.data(), (int)n);
    return out;
}
static std::vector<uint8_t> deriveKeyPBKDF2_OpenSSL(const std::string& pass, const std::vector<uint8_t>& salt, uint32_t iterations, size_t keyLen) {
    std::vector<uint8_t> key(keyLen);
    PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt.data(), (int)salt.size(), iterations, EVP_sha256(), (int)keyLen, key.data());
    return key;
}
static std::vector<uint8_t> aesGcmEncryptOpenSSL(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce,
                                                 const std::vector<uint8_t>& aad, const std::vector<uint8_t>& plaintext,
                                                 std::vector<uint8_t>& tagOut) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)nonce.size(), nullptr);
    EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data());
    int outlen = 0;
    if (!aad.empty()) EVP_EncryptUpdate(ctx, nullptr, &outlen, aad.data(), (int)aad.size());
    std::vector<uint8_t> ct(plaintext.size());
    EVP_EncryptUpdate(ctx, ct.data(), &outlen, plaintext.data(), (int)plaintext.size());
    int tmplen = 0;
    EVP_EncryptFinal_ex(ctx, nullptr, &tmplen);
    tagOut.resize(16);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)tagOut.size(), tagOut.data());
    EVP_CIPHER_CTX_free(ctx);
    return ct;
}
static std::vector<uint8_t> aesGcmDecryptOpenSSL(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce,
                                                 const std::vector<uint8_t>& aad, const std::vector<uint8_t>& ciphertext,
                                                 const std::vector<uint8_t>& tag) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)nonce.size(), nullptr);
    EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data());
    int outlen = 0;
    if (!aad.empty()) EVP_DecryptUpdate(ctx, nullptr, &outlen, aad.data(), (int)aad.size());
    std::vector<uint8_t> pt(ciphertext.size());
    EVP_DecryptUpdate(ctx, pt.data(), &outlen, ciphertext.data(), (int)ciphertext.size());
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)tag.size(), const_cast<uint8_t*>(tag.data()));
    int ok = EVP_DecryptFinal_ex(ctx, nullptr, &outlen);
    EVP_CIPHER_CTX_free(ctx);
    if (ok != 1) throw std::runtime_error("OpenSSL: decrypt failed");
    return pt;
}
static std::vector<uint8_t> hmacSha256OpenSSL(const std::string& key, const std::string& msg) {
    unsigned int len = 32;
    std::vector<uint8_t> out(len);
    HMAC(EVP_sha256(), key.data(), (int)key.size(), reinterpret_cast<const unsigned char*>(msg.data()), (int)msg.size(), out.data(), &len);
    out.resize(len);
    return out;
}
#endif

void registerNativeIO(const std::shared_ptr<Environment>& globals) {
    globals->define("input", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canInput()) {
                throw std::runtime_error("Input disabled by sandbox");
            }
            if (isString(args[0])) {
                std::cout << asString(args[0]);
            }
            std::string line;
            std::getline(std::cin, line);
            auto sv = StringPool::intern(line);
            return stringValue(sv.data());
        },
        "input"
    ));

    globals->define("readFile", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileRead()) {
                throw std::runtime_error("File read disabled by sandbox");
            }
            if (!isString(args[0])) {
                throw std::runtime_error("readFile() requires a string path");
            }
            std::string path = asString(args[0]);
            if (globals->defaultEncryptedIO() && !globals->ioEncPass().empty()) {
                std::ifstream f(path, std::ios::binary);
                if (f) {
                    std::string magic(5, '\0');
                    f.read(magic.data(), 5);
                    if (magic == "VENC1") {
#ifdef _WIN32
                        std::vector<uint8_t> salt(16), nonce(12), tag(16);
                        f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
                        f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
                        f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
                        std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                        auto key = deriveKeyPBKDF2(globals->ioEncPass(), salt, 100000, 32);
                        const std::string magicStr = "VENC1";
                        std::vector<uint8_t> aad(magicStr.begin(), magicStr.end());
                        auto pt = aesGcmDecrypt(key, nonce, aad, ct, tag);
                        auto sv = StringPool::intern(std::string(pt.begin(), pt.end()));
                        return stringValue(sv.data());
#elif defined(CLAW_HAS_OPENSSL)
                        std::vector<uint8_t> salt(16), nonce(12), tag(16);
                        f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
                        f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
                        f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
                        std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                        auto key = deriveKeyPBKDF2_OpenSSL(globals->ioEncPass(), salt, 100000, 32);
                        const std::string magicStr = "VENC1";
                        std::vector<uint8_t> aad(magicStr.begin(), magicStr.end());
                        auto pt = aesGcmDecryptOpenSSL(key, nonce, aad, ct, tag);
                        auto sv = StringPool::intern(std::string(pt.begin(), pt.end()));
                        return stringValue(sv.data());
#else
                        throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
                    }
                }
            }
            std::ifstream file(path);
            if (!file) {
                throw std::runtime_error("Could not open file: " + path);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto sv = StringPool::intern(buffer.str());
            return stringValue(sv.data());
        },
        "readFile"
    ));

    globals->define("writeFile", std::make_shared<NativeFunction>(
        2,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileWrite()) {
                throw std::runtime_error("File write disabled by sandbox");
            }
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("writeFile() requires string path and content");
            }
            std::string path = asString(args[0]);
            if (globals->defaultEncryptedIO() && !globals->ioEncPass().empty()) {
#ifdef _WIN32
                std::string content = asString(args[1]);
                std::vector<uint8_t> salt = randomBytes(16);
                std::vector<uint8_t> nonce = randomBytes(12);
                auto key = deriveKeyPBKDF2(globals->ioEncPass(), salt, 100000, 32);
                const std::string magic = "VENC1";
                std::vector<uint8_t> aad(magic.begin(), magic.end());
                std::vector<uint8_t> tag;
                auto ct = aesGcmEncrypt(key, nonce, aad, std::vector<uint8_t>(content.begin(), content.end()), tag);
                std::ofstream of(path, std::ios::binary);
                if (!of) throw std::runtime_error("Could not open file for writing: " + path);
                of.write(magic.data(), (std::streamsize)magic.size());
                of.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
                of.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
                of.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
                of.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
                return nilValue();
#elif defined(CLAW_HAS_OPENSSL)
                std::string content = asString(args[1]);
                std::vector<uint8_t> salt = randomBytesOpenSSL(16);
                std::vector<uint8_t> nonce = randomBytesOpenSSL(12);
                auto key = deriveKeyPBKDF2_OpenSSL(globals->ioEncPass(), salt, 100000, 32);
                const std::string magic = "VENC1";
                std::vector<uint8_t> aad(magic.begin(), magic.end());
                std::vector<uint8_t> tag;
                auto ct = aesGcmEncryptOpenSSL(key, nonce, aad, std::vector<uint8_t>(content.begin(), content.end()), tag);
                std::ofstream of(path, std::ios::binary);
                if (!of) throw std::runtime_error("Could not open file for writing: " + path);
                of.write(magic.data(), (std::streamsize)magic.size());
                of.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
                of.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
                of.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
                of.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
                return nilValue();
#else
                throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
            }
            std::ofstream file(path);
            if (!file) {
                throw std::runtime_error("Could not open file for writing: " + path);
            }
            file << asString(args[1]);
            return nilValue();
        },
        "writeFile"
    ));

    globals->define("appendFile", std::make_shared<NativeFunction>(
        2,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileWrite()) {
                throw std::runtime_error("File write disabled by sandbox");
            }
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("appendFile() requires string path and content");
            }
            std::string path = asString(args[0]);
            if (globals->defaultEncryptedIO() && !globals->ioEncPass().empty()) {
                std::ifstream f(path, std::ios::binary);
                std::string existing;
                if (f) {
                    std::string magic(5, '\0');
                    f.read(magic.data(), 5);
                    if (magic == "VENC1") {
#ifdef _WIN32
                        std::vector<uint8_t> salt(16), nonce(12), tag(16);
                        f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
                        f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
                        f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
                        std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                        auto key = deriveKeyPBKDF2(globals->ioEncPass(), salt, 100000, 32);
                        const std::string magicStr = "VENC1";
                        std::vector<uint8_t> aad(magicStr.begin(), magicStr.end());
                        auto pt = aesGcmDecrypt(key, nonce, aad, ct, tag);
                        existing.assign(reinterpret_cast<const char*>(pt.data()), pt.size());
#elif defined(CLAW_HAS_OPENSSL)
                        std::vector<uint8_t> salt(16), nonce(12), tag(16);
                        f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
                        f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
                        f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
                        std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                        auto key = deriveKeyPBKDF2_OpenSSL(globals->ioEncPass(), salt, 100000, 32);
                        const std::string magicStr = "VENC1";
                        std::vector<uint8_t> aad(magicStr.begin(), magicStr.end());
                        auto pt = aesGcmDecryptOpenSSL(key, nonce, aad, ct, tag);
                        existing.assign(reinterpret_cast<const char*>(pt.data()), pt.size());
#else
                        throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
                    } else {
                        f.seekg(0, std::ios::beg);
                        std::stringstream buffer;
                        buffer << f.rdbuf();
                        existing = buffer.str();
                    }
                }
                std::string appended = existing + asString(args[1]);
#ifdef _WIN32
                std::vector<uint8_t> salt = randomBytes(16);
                std::vector<uint8_t> nonce = randomBytes(12);
                auto key = deriveKeyPBKDF2(globals->ioEncPass(), salt, 100000, 32);
                const std::string magic = "VENC1";
                std::vector<uint8_t> aad(magic.begin(), magic.end());
                std::vector<uint8_t> tag;
                auto ct = aesGcmEncrypt(key, nonce, aad, std::vector<uint8_t>(appended.begin(), appended.end()), tag);
                std::ofstream of(path, std::ios::binary);
                if (!of) throw std::runtime_error("Could not open file for writing: " + path);
                of.write(magic.data(), (std::streamsize)magic.size());
                of.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
                of.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
                of.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
                of.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
                return boolValue(true);
#elif defined(CLAW_HAS_OPENSSL)
                std::vector<uint8_t> salt = randomBytesOpenSSL(16);
                std::vector<uint8_t> nonce = randomBytesOpenSSL(12);
                auto key = deriveKeyPBKDF2_OpenSSL(globals->ioEncPass(), salt, 100000, 32);
                const std::string magic = "VENC1";
                std::vector<uint8_t> aad(magic.begin(), magic.end());
                std::vector<uint8_t> tag;
                auto ct = aesGcmEncryptOpenSSL(key, nonce, aad, std::vector<uint8_t>(appended.begin(), appended.end()), tag);
                std::ofstream of(path, std::ios::binary);
                if (!of) throw std::runtime_error("Could not open file for writing: " + path);
                of.write(magic.data(), (std::streamsize)magic.size());
                of.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
                of.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
                of.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
                of.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
                return boolValue(true);
#else
                throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
            }
            std::ofstream file(path, std::ios::app);
            if (!file) {
                throw std::runtime_error("Could not open file for appending: " + path);
            }
            file << asString(args[1]);
            return boolValue(true);
        },
        "appendFile"
    ));

    globals->define("fileExists", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileRead()) {
                throw std::runtime_error("File read disabled by sandbox");
            }
            if (!isString(args[0])) {
                throw std::runtime_error("fileExists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return boolValue(file.good());
        },
        "fileExists"
    ));

    globals->define("exists", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileRead()) {
                throw std::runtime_error("File read disabled by sandbox");
            }
            if (!isString(args[0])) {
                throw std::runtime_error("exists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return boolValue(file.good());
        },
        "exists"
    ));

    globals->define("deleteFile", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileDelete()) {
                throw std::runtime_error("File delete disabled by sandbox");
            }
            if (!isString(args[0])) {
                throw std::runtime_error("deleteFile() requires a string path");
            }
            std::string path = asString(args[0]);
            if (std::remove(path.c_str()) == 0) {
                return boolValue(true);
            } else {
                return boolValue(false);
            }
        },
        "deleteFile"
    ));

    globals->define("fileSize", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileRead()) {
                throw std::runtime_error("File read disabled by sandbox");
            }
            if (!isString(args[0])) {
                throw std::runtime_error("fileSize() requires a string path");
            }
            std::string path = asString(args[0]);
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) {
                throw std::runtime_error("Could not open file: " + path);
            }
            return numberToValue(static_cast<double>(file.tellg()));
        },
        "fileSize"
    ));

    globals->define("policyReload", std::make_shared<NativeFunction>(
        0,
        [globals](const std::vector<Value>&) -> Value {
            std::ifstream f(".voltsec");
            if (!f) return boolValue(false);
            std::unordered_map<std::string, std::string> kv;
            std::string line;
            while (std::getline(f, line)) {
                auto t = trim(line);
                if (t.empty() || t[0] == '#') continue;
                auto eq = t.find('=');
                if (eq == std::string::npos) continue;
                kv[trim(t.substr(0, eq))] = trim(t.substr(eq + 1));
            }
            auto sv = [&](const std::string& k, const std::string& def) {
                auto it = kv.find(k);
                return it == kv.end() ? def : it->second;
            };
            auto allow = [&](const std::string& v){ return v == "allow" || v == "true" || v == "1"; };
            std::string sandbox = sv("sandbox", "");
            if (sandbox == "strict") globals->setSandbox(Environment::SandboxMode::Strict);
            else if (sandbox == "network") globals->setSandbox(Environment::SandboxMode::Network);
            else if (sandbox == "full") globals->setSandbox(Environment::SandboxMode::Full);
            std::string fr = sv("file.read", "");
            std::string fw = sv("file.write", "");
            std::string fd = sv("file.delete", "");
            std::string in = sv("input", "");
            std::string out = sv("output", "");
            std::string net = sv("network", "");
            if (!fr.empty()) globals->setFileReadAllowed(allow(fr));
            if (!fw.empty()) globals->setFileWriteAllowed(allow(fw));
            if (!fd.empty()) globals->setFileDeleteAllowed(allow(fd));
            if (!in.empty()) globals->setInputAllowed(allow(in));
            if (!out.empty()) globals->setOutputAllowed(allow(out));
            if (!net.empty()) globals->setNetworkAllowed(allow(net));
            std::string logp = sv("log.path", "");
            std::string logk = sv("log.hmac", "");
            std::string logm = sv("log.meta.required", "");
            if (!logp.empty()) globals->setLogPath(logp);
            if (!logk.empty()) globals->setLogHmacKey(logk);
            if (!logm.empty()) globals->setLogMetaRequired(allow(logm));
            return boolValue(true);
        },
        "policyReload"
    ));

    globals->define("logWrite", std::make_shared<NativeFunction>(
        -1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileWrite()) {
                throw std::runtime_error("File write disabled by sandbox");
            }
            if (args.size() == 0 || !isString(args[0])) throw std::runtime_error("logWrite(message[, metadata]) requires string message");
            if (globals->logMetaRequired() && args.size() < 2) throw std::runtime_error("Log metadata required by policy");
            std::string msg = asString(args[0]);
            std::string path = globals->logPath();
            std::string key = globals->logHmacKey();
            std::string metaJson;
            if (args.size() >= 2) {
                if (!isHashMap(args[1])) throw std::runtime_error("logWrite metadata must be a map");
                auto m = asHashMap(args[1]);
                std::vector<std::string> keys;
                keys.reserve(m->data.size());
                for (const auto& kv : m->data) { keys.emplace_back(kv.first); }
                std::sort(keys.begin(), keys.end());
                std::ostringstream oss;
                oss << "{";
                bool first = true;
                for (auto& k : keys) {
                    if (!first) oss << ",";
                    first = false;
                    Value vv = m->get(k);
                    if (isNil(vv)) continue;
                    auto escape = [](const std::string& s){
                        std::ostringstream e;
                        for (char c : s) {
                            if (c == '"' || c == '\\') { e << '\\' << c; }
                            else if (c == '\n') { e << "\\n"; }
                            else if (c == '\r') { e << "\\r"; }
                            else if (c == '\t') { e << "\\t"; }
                            else { e << c; }
                        }
                        return e.str();
                    };
                    oss << "\"" << escape(k) << "\":";
                    if (isString(vv)) {
                        oss << "\"" << escape(asString(vv)) << "\"";
                    } else if (isNumber(vv)) {
                        oss << valueToString(vv);
                    } else if (isBool(vv)) {
                        oss << (asBool(vv) ? "true" : "false");
                    } else if (isNil(vv)) {
                        oss << "null";
                    } else {
                        oss << "\"" << escape(valueToString(vv)) << "\"";
                    }
                }
                oss << "}";
                metaJson = oss.str();
            }
            std::ofstream file(path, std::ios::app | std::ios::binary);
            if (!file) throw std::runtime_error("Could not open log file: " + path);
            if (key.empty()) {
                if (metaJson.empty()) {
                    file << msg << "\n";
                } else {
                    file << msg << "|" << metaJson << "\n";
                }
            } else {
#ifdef _WIN32
                BCRYPT_ALG_HANDLE h = nullptr;
                if (BCryptOpenAlgorithmProvider(&h, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG) != 0) {
                    throw std::runtime_error("BCrypt: SHA256 HMAC provider open failed");
                }
                BCRYPT_HASH_HANDLE hh = nullptr;
                if (BCryptCreateHash(h, &hh, nullptr, 0, reinterpret_cast<UCHAR*>(const_cast<char*>(key.data())), (ULONG)key.size(), 0) != 0) {
                    BCryptCloseAlgorithmProvider(h, 0);
                    throw std::runtime_error("BCrypt: HMAC create failed");
                }
                std::string covered = metaJson.empty() ? msg : (msg + "|" + metaJson);
                if (BCryptHashData(hh, reinterpret_cast<UCHAR*>(const_cast<char*>(covered.data())), (ULONG)covered.size(), 0) != 0) {
                    BCryptDestroyHash(hh);
                    BCryptCloseAlgorithmProvider(h, 0);
                    throw std::runtime_error("BCrypt: Hash data failed");
                }
                std::vector<uint8_t> out(32);
                if (BCryptFinishHash(hh, out.data(), (ULONG)out.size(), 0) != 0) {
                    BCryptDestroyHash(hh);
                    BCryptCloseAlgorithmProvider(h, 0);
                    throw std::runtime_error("BCrypt: Finish hash failed");
                }
                BCryptDestroyHash(hh);
                BCryptCloseAlgorithmProvider(h, 0);
                file << msg << "|";
                static const char* hexd = "0123456789abcdef";
                for (auto b : out) { file << hexd[b >> 4] << hexd[b & 0xF]; }
                if (!metaJson.empty()) file << "|" << metaJson;
                file << "\n";
#elif defined(CLAW_HAS_OPENSSL)
                std::string covered = metaJson.empty() ? msg : (msg + "|" + metaJson);
                auto mac = hmacSha256OpenSSL(key, covered);
                file << msg << "|";
                static const char* hexd = "0123456789abcdef";
                for (auto b : mac) { file << hexd[b >> 4] << hexd[b & 0xF]; }
                if (!metaJson.empty()) file << "|" << metaJson;
                file << "\n";
#else
                throw std::runtime_error("HMAC not supported on this platform");
#endif
            }
            return boolValue(true);
        },
        "logWrite"
    ));

    globals->define("tlsGet", std::make_shared<NativeFunction>(
        -1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canNetwork()) throw std::runtime_error("Network disabled by sandbox");
            if (args.empty() || !isString(args[0])) throw std::runtime_error("tlsGet(url[, headers]) requires string url");
            std::string url = asString(args[0]);
#ifdef CLAW_HAS_OPENSSL
            std::vector<std::pair<std::string,std::string>> headers;
            if (args.size() >= 2) {
                if (!isHashMap(args[1])) throw std::runtime_error("tlsGet headers must be a map");
                auto m = asHashMap(args[1]);
                headers.reserve(m->data.size());
                for (const auto& kv : m->data) {
                    headers.emplace_back(kv.first, valueToString(kv.second));
                }
            }
#endif
#ifndef CLAW_HAS_OPENSSL
            throw std::runtime_error("TLS not supported (OpenSSL unavailable)");
#else
            if (url.rfind("https://", 0) != 0) throw std::runtime_error("Only https URLs are supported");
            std::string rest = url.substr(8);
            std::string hostport, path = "/";
            size_t slash = rest.find('/');
            if (slash == std::string::npos) hostport = rest;
            else { hostport = rest.substr(0, slash); path = rest.substr(slash); }
            std::string host = hostport;
            int port = 443;
            size_t colon = hostport.find(':');
            if (colon != std::string::npos) {
                host = hostport.substr(0, colon);
                port = std::stoi(hostport.substr(colon + 1));
            }
            OPENSSL_init_ssl(0, nullptr);
            SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
            #ifdef TLS1_3_VERSION
            SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
            #endif
            BIO* bio = BIO_new_ssl_connect(ctx);
            std::string hp = host + ":" + std::to_string(port);
            BIO_set_conn_hostname(bio, hp.c_str());
            SSL* ssl = nullptr;
            BIO_get_ssl(bio, &ssl);
            SSL_set_tlsext_host_name(ssl, host.c_str());
            if (BIO_do_connect(bio) <= 0) {
                BIO_free_all(bio);
                SSL_CTX_free(ctx);
                throw std::runtime_error("TLS connect failed");
            }
            if (BIO_do_handshake(bio) <= 0) {
                BIO_free_all(bio);
                SSL_CTX_free(ctx);
                throw std::runtime_error("TLS handshake failed");
            }
            std::ostringstream req;
            req << "GET " << path << " HTTP/1.1\r\n"
                << "Host: " << host << "\r\n"
                << "Connection: close\r\n";
            for (auto& kv : headers) {
                req << kv.first << ": " << kv.second << "\r\n";
            }
            req << "\r\n";
            std::string reqStr = req.str();
            BIO_write(bio, reqStr.data(), (int)reqStr.size());
            std::string resp;
            char buf[4096];
            int n = 0;
            while ((n = BIO_read(bio, buf, sizeof(buf))) > 0) {
                resp.append(buf, buf + n);
            }
            BIO_free_all(bio);
            SSL_CTX_free(ctx);
            size_t sep = resp.find("\r\n\r\n");
            std::string body = (sep == std::string::npos) ? resp : resp.substr(sep + 4);
            auto sv = StringPool::intern(body);
            return stringValue(sv.data());
#endif
        },
        "tlsGet"
    ));

    globals->define("tlsPost", std::make_shared<NativeFunction>(
        -1,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canNetwork()) throw std::runtime_error("Network disabled by sandbox");
            if (args.size() < 2 || !isString(args[0]) || !isString(args[1])) throw std::runtime_error("tlsPost(url, body[, headers]) requires strings");
            std::string url = asString(args[0]);
            std::string body = asString(args[1]);
#ifdef CLAW_HAS_OPENSSL
            std::vector<std::pair<std::string,std::string>> headers;
            if (args.size() >= 3) {
                if (!isHashMap(args[2])) throw std::runtime_error("tlsPost headers must be a map");
                auto m = asHashMap(args[2]);
                headers.reserve(m->data.size());
                for (const auto& kv : m->data) {
                    headers.emplace_back(kv.first, valueToString(kv.second));
                }
            }
#endif
#ifndef CLAW_HAS_OPENSSL
            throw std::runtime_error("TLS not supported (OpenSSL unavailable)");
#else
            if (url.rfind("https://", 0) != 0) throw std::runtime_error("Only https URLs are supported");
            std::string rest = url.substr(8);
            std::string hostport, path = "/";
            size_t slash = rest.find('/');
            if (slash == std::string::npos) hostport = rest;
            else { hostport = rest.substr(0, slash); path = rest.substr(slash); }
            std::string host = hostport;
            int port = 443;
            size_t colon = hostport.find(':');
            if (colon != std::string::npos) {
                host = hostport.substr(0, colon);
                port = std::stoi(hostport.substr(colon + 1));
            }
            OPENSSL_init_ssl(0, nullptr);
            SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
            #ifdef TLS1_3_VERSION
            SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
            #endif
            BIO* bio = BIO_new_ssl_connect(ctx);
            std::string hp = host + ":" + std::to_string(port);
            BIO_set_conn_hostname(bio, hp.c_str());
            SSL* ssl = nullptr;
            BIO_get_ssl(bio, &ssl);
            SSL_set_tlsext_host_name(ssl, host.c_str());
            if (BIO_do_connect(bio) <= 0) {
                BIO_free_all(bio);
                SSL_CTX_free(ctx);
                throw std::runtime_error("TLS connect failed");
            }
            if (BIO_do_handshake(bio) <= 0) {
                BIO_free_all(bio);
                SSL_CTX_free(ctx);
                throw std::runtime_error("TLS handshake failed");
            }
            std::ostringstream req;
            req << "POST " << path << " HTTP/1.1\r\n"
                << "Host: " << host << "\r\n"
                << "Connection: close\r\n"
                << "Content-Length: " << body.size() << "\r\n";
            bool hasCT = false;
            auto iequals = [](const std::string& a, const std::string& b){
                if (a.size() != b.size()) return false;
                for (size_t i = 0; i < a.size(); ++i) {
                    unsigned char ca = static_cast<unsigned char>(a[i]);
                    unsigned char cb = static_cast<unsigned char>(b[i]);
                    if (std::tolower(ca) != std::tolower(cb)) return false;
                }
                return true;
            };
            for (auto& kv : headers) {
                if (iequals(kv.first, "Content-Type")) hasCT = true;
                req << kv.first << ": " << kv.second << "\r\n";
            }
            if (!hasCT) req << "Content-Type: application/octet-stream\r\n";
            req << "\r\n" << body;
            std::string reqStr = req.str();
            BIO_write(bio, reqStr.data(), (int)reqStr.size());
            std::string resp;
            char buf[4096];
            int n = 0;
            while ((n = BIO_read(bio, buf, sizeof(buf))) > 0) {
                resp.append(buf, buf + n);
            }
            BIO_free_all(bio);
            SSL_CTX_free(ctx);
            size_t sep = resp.find("\r\n\r\n");
            std::string respBody = (sep == std::string::npos) ? resp : resp.substr(sep + 4);
            auto sv = StringPool::intern(respBody);
            return stringValue(sv.data());
#endif
        },
        "tlsPost"
    ));

    globals->define("writeFileEnc", std::make_shared<NativeFunction>(
        3,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileWrite()) {
                throw std::runtime_error("File write disabled by sandbox");
            }
            if (!isString(args[0]) || !isString(args[1]) || !isString(args[2])) {
                throw std::runtime_error("writeFileEnc(path, content, passphrase) requires all string args");
            }
#ifdef _WIN32
            std::string path = asString(args[0]);
            std::string content = asString(args[1]);
            std::string pass = asString(args[2]);
            std::vector<uint8_t> salt = randomBytes(16);
            std::vector<uint8_t> nonce = randomBytes(12);
            auto key = deriveKeyPBKDF2(pass, salt, 100000, 32);
            const std::string magic = "VENC1";
            std::vector<uint8_t> aad(magic.begin(), magic.end());
            std::vector<uint8_t> tag;
            auto ct = aesGcmEncrypt(key, nonce, aad, std::vector<uint8_t>(content.begin(), content.end()), tag);
            std::ofstream file(path, std::ios::binary);
            if (!file) throw std::runtime_error("Could not open file for writing: " + path);
            file.write(magic.data(), (std::streamsize)magic.size());
            file.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
            file.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
            file.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
            file.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
            return boolValue(true);
#elif defined(CLAW_HAS_OPENSSL)
            std::string path = asString(args[0]);
            std::string content = asString(args[1]);
            std::string pass = asString(args[2]);
            std::vector<uint8_t> salt = randomBytesOpenSSL(16);
            std::vector<uint8_t> nonce = randomBytesOpenSSL(12);
            auto key = deriveKeyPBKDF2_OpenSSL(pass, salt, 100000, 32);
            const std::string magic = "VENC1";
            std::vector<uint8_t> aad(magic.begin(), magic.end());
            std::vector<uint8_t> tag;
            auto ct = aesGcmEncryptOpenSSL(key, nonce, aad, std::vector<uint8_t>(content.begin(), content.end()), tag);
            std::ofstream file(path, std::ios::binary);
            if (!file) throw std::runtime_error("Could not open file for writing: " + path);
            file.write(magic.data(), (std::streamsize)magic.size());
            file.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
            file.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
            file.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
            file.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
            return boolValue(true);
#else
            throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
        },
        "writeFileEnc"
    ));

    globals->define("readFileEnc", std::make_shared<NativeFunction>(
        2,
        [globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileRead()) {
                throw std::runtime_error("File read disabled by sandbox");
            }
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("readFileEnc(path, passphrase) requires string args");
            }
#ifdef _WIN32
            std::string path = asString(args[0]);
            std::string pass = asString(args[1]);
            std::ifstream file(path, std::ios::binary);
            if (!file) throw std::runtime_error("Could not open file: " + path);
            std::string magic(5, '\0');
            file.read(magic.data(), 5);
            if (magic != "VENC1") throw std::runtime_error("Invalid encrypted file format");
            std::vector<uint8_t> salt(16), nonce(12), tag(16);
            file.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
            file.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
            file.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
            std::vector<uint8_t> ct((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            auto key = deriveKeyPBKDF2(pass, salt, 100000, 32);
            const std::string magicStr = "VENC1";
            std::vector<uint8_t> aad(magicStr.begin(), magicStr.end());
            auto pt = aesGcmDecrypt(key, nonce, aad, ct, tag);
            auto sv = StringPool::intern(std::string(pt.begin(), pt.end()));
            return stringValue(sv.data());
#elif defined(CLAW_HAS_OPENSSL)
            std::string path = asString(args[0]);
            std::string pass = asString(args[1]);
            std::ifstream file(path, std::ios::binary);
            if (!file) throw std::runtime_error("Could not open file: " + path);
            std::string magic(5, '\0');
            file.read(magic.data(), 5);
            if (magic != "VENC1") throw std::runtime_error("Invalid encrypted file format");
            std::vector<uint8_t> salt(16), nonce(12), tag(16);
            file.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
            file.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
            file.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
            std::vector<uint8_t> ct((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            auto key = deriveKeyPBKDF2_OpenSSL(pass, salt, 100000, 32);
            const std::string magicStr = "VENC1";
            std::vector<uint8_t> aad(magicStr.begin(), magicStr.end());
            auto pt = aesGcmDecryptOpenSSL(key, nonce, aad, ct, tag);
            auto sv = StringPool::intern(std::string(pt.begin(), pt.end()));
            return stringValue(sv.data());
#else
            throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
        },
        "readFileEnc"
    ));
}

} // namespace claw
