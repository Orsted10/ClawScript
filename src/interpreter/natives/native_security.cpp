#include "interpreter/natives/native_security.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include <string>
#include <sstream>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#include <winreg.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif
#ifdef CLAW_HAS_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif
namespace claw {
static bool dbgPresent() {
#ifdef _WIN32
    return IsDebuggerPresent() != 0;
#else
    return false;
#endif
}
static bool vmDetectedBasic() {
#ifdef _WIN32
    bool vm = false;
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buf[256]; DWORD sz = sizeof(buf);
        if (RegQueryValueExA(hKey, "SystemBiosVersion", nullptr, nullptr, reinterpret_cast<LPBYTE>(buf), &sz) == ERROR_SUCCESS) {
            std::string s(buf, buf + strnlen(buf, sizeof(buf)));
            if (s.find("VMware") != std::string::npos || s.find("VirtualBox") != std::string::npos || s.find("QEMU") != std::string::npos) vm = true;
        }
        RegCloseKey(hKey);
    }
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0x40000000);
    char hv[13]; memcpy(hv, &cpuInfo[1], 4); memcpy(hv + 4, &cpuInfo[2], 4); memcpy(hv + 8, &cpuInfo[3], 4); hv[12] = 0;
    std::string hvstr(hv);
    if (!hvstr.empty()) vm = true;
    return vm;
#else
    return false;
#endif
}
static std::string cryptoAlgoNormalize(const std::string& a) {
    if (a == "AES" || a == "AES_GCM" || a == "aes" || a == "aes_gcm") return "AES_GCM";
    if (a == "CHACHA" || a == "CHACHA20_POLY1305" || a == "chacha" || a == "chacha20_poly1305") return "CHACHA20_POLY1305";
    if (a == "PQC_AES_GCM" || a == "pqc_aes_gcm") return "PQC_AES_GCM";
    if (a == "PQC_CHACHA20_POLY1305" || a == "pqc_chacha20_poly1305") return "PQC_CHACHA20_POLY1305";
    return "AES_GCM";
}
static std::vector<uint8_t> kdf_win(const std::string& pass, const std::vector<uint8_t>& salt, uint32_t iterations, size_t keyLen) {
#ifdef _WIN32
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
#else
    return {};
#endif
}
static std::vector<uint8_t> aes_gcm_dec_win(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce,
                                            const std::vector<uint8_t>& aad, const std::vector<uint8_t>& ciphertext,
                                            const std::vector<uint8_t>& tag) {
#ifdef _WIN32
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
    if (st != 0) throw std::runtime_error("BCrypt: decrypt failed");
    return plaintext;
#else
    return {};
#endif
}
static std::vector<uint8_t> kdf_openssl(const std::string& pass, const std::vector<uint8_t>& salt, uint32_t iterations, size_t keyLen) {
#ifdef CLAW_HAS_OPENSSL
    std::vector<uint8_t> key(keyLen);
    PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt.data(), (int)salt.size(), iterations, EVP_sha256(), (int)keyLen, key.data());
    return key;
#else
    return {};
#endif
}
static std::vector<uint8_t> aes_gcm_dec_openssl(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce,
                                                 const std::vector<uint8_t>& aad, const std::vector<uint8_t>& ciphertext,
                                                 const std::vector<uint8_t>& tag) {
#ifdef CLAW_HAS_OPENSSL
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
#else
    return {};
#endif
}
void registerNativeSecurity(const std::shared_ptr<Environment>& globals, Interpreter& I) {
    globals->define("isDebuggerPresent", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value { return boolValue(dbgPresent()); },
        "isDebuggerPresent"
    ));
    globals->define("vmDetected", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value { return boolValue(vmDetectedBasic()); },
        "vmDetected"
    ));
    globals->define("antiDebugEnforce", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            bool on = args.empty() ? true : (isBool(args[0]) ? asBool(args[0]) : true);
            globals->setAntiDebugEnforced(on);
            if (on && dbgPresent()) throw std::runtime_error("Debugger detected");
            return boolValue(on);
        },
        "antiDebugEnforce"
    ));
    globals->define("cryptoPrefer", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            if (args.empty() || !isString(args[0])) throw std::runtime_error("cryptoPrefer(algo) requires string");
            std::string aIn = asString(args[0]);
            std::string a = cryptoAlgoNormalize(aIn);
            globals->setCryptoPreferred(a);
            return stringValue(StringPool::intern(a).data());
        },
        "cryptoPrefer"
    ));
    globals->define("dynamicCodeEncryption", std::make_shared<NativeFunction>(
        1,
        [globals](const std::vector<Value>& args) -> Value {
            bool on = args.empty() ? true : (isBool(args[0]) ? asBool(args[0]) : true);
            globals->setDynamicCodeEncryption(on);
            return boolValue(on);
        },
        "dynamicCodeEncryption"
    ));
    globals->define("execEncFile", std::make_shared<NativeFunction>(
        2,
        [&I, globals](const std::vector<Value>& args) -> Value {
            if (!globals->canFileRead()) throw std::runtime_error("File read disabled by sandbox");
            if (!isString(args[0]) || !isString(args[1])) throw std::runtime_error("execEncFile(path, pass) requires strings");
            std::string path = asString(args[0]);
            std::string pass = asString(args[1]);
            std::ifstream f(path, std::ios::binary);
            if (!f) throw std::runtime_error("Could not open file");
            std::string magic(5, '\0');
            f.read(magic.data(), 5);
            if (magic != "VENC1") throw std::runtime_error("Invalid encrypted file format");
#ifdef _WIN32
            std::vector<uint8_t> salt(16), nonce(12), tag(16);
            f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
            f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
            f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
            std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            auto key = kdf_win(pass, salt, 100000, 32);
            std::vector<uint8_t> aad(magic.begin(), magic.end());
            auto pt = aes_gcm_dec_win(key, nonce, aad, ct, tag);
            std::string src(pt.begin(), pt.end());
#elif defined(CLAW_HAS_OPENSSL)
            std::vector<uint8_t> salt(16), nonce(12), tag(16);
            f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
            f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
            f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
            std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            auto key = kdf_openssl(pass, salt, 100000, 32);
            std::vector<uint8_t> aad(magic.begin(), magic.end());
            auto pt = aes_gcm_dec_openssl(key, nonce, aad, ct, tag);
            std::string src(pt.begin(), pt.end());
#else
            throw std::runtime_error("Encrypted I/O not supported on this platform");
#endif
            Lexer lex(src);
            auto toks = lex.tokenize();
            Parser par(toks);
            auto stmts = par.parseProgram();
            if (par.hadError()) throw std::runtime_error("Parse error");
            I.execute(stmts);
            return nilValue();
        },
        "execEncFile"
    ));
#ifdef CLAW_HAS_OPENSSL
    globals->define("writeFileEncAlgo", std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1]) || !isString(args[2]) || !isString(args[3])) throw std::runtime_error("writeFileEncAlgo(path, content, pass, algo)");
            std::string algo = cryptoAlgoNormalize(asString(args[3]));
            if (algo == "PQC_CHACHA20_POLY1305") {
#ifndef CLAW_HAS_PQC
                throw std::runtime_error("PQC CHACHA20_POLY1305 not available (build without ENABLE_PQC)");
#endif
                algo = "CHACHA20_POLY1305";
            }
            if (algo != "CHACHA20_POLY1305") throw std::runtime_error("Only CHACHA20_POLY1305 here");
            std::string path = asString(args[0]);
            std::string content = asString(args[1]);
            std::string pass = asString(args[2]);
            std::vector<uint8_t> salt(16), nonce(12);
            RAND_bytes(salt.data(), (int)salt.size());
            RAND_bytes(nonce.data(), (int)nonce.size());
            std::vector<uint8_t> key(32);
            PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt.data(), (int)salt.size(), 100000, EVP_sha256(), 32, key.data());
            std::vector<uint8_t> tag(16);
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, nullptr, nullptr);
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, (int)nonce.size(), nullptr);
            EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data());
            std::vector<uint8_t> ct(content.size());
            int outlen = 0;
            EVP_EncryptUpdate(ctx, ct.data(), &outlen, reinterpret_cast<const uint8_t*>(content.data()), (int)content.size());
            int tmplen = 0;
            EVP_EncryptFinal_ex(ctx, nullptr, &tmplen);
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, (int)tag.size(), tag.data());
            EVP_CIPHER_CTX_free(ctx);
            std::ofstream of(path, std::ios::binary);
            if (!of) throw std::runtime_error("Could not open file");
            of.write("VENC1", 5);
            of.write(reinterpret_cast<const char*>(salt.data()), (std::streamsize)salt.size());
            of.write(reinterpret_cast<const char*>(nonce.data()), (std::streamsize)nonce.size());
            of.write(reinterpret_cast<const char*>(tag.data()), (std::streamsize)tag.size());
            of.write(reinterpret_cast<const char*>(ct.data()), (std::streamsize)ct.size());
            return boolValue(true);
        },
        "writeFileEncAlgo"
    ));
    globals->define("readFileEncAlgo", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1]) || !isString(args[2])) throw std::runtime_error("readFileEncAlgo(path, pass, algo)");
            std::string algo = cryptoAlgoNormalize(asString(args[2]));
            if (algo == "PQC_CHACHA20_POLY1305") {
#ifndef CLAW_HAS_PQC
                throw std::runtime_error("PQC CHACHA20_POLY1305 not available (build without ENABLE_PQC)");
#endif
                algo = "CHACHA20_POLY1305";
            }
            if (algo != "CHACHA20_POLY1305") throw std::runtime_error("Only CHACHA20_POLY1305 here");
            std::string path = asString(args[0]);
            std::string pass = asString(args[1]);
            std::ifstream f(path, std::ios::binary);
            if (!f) throw std::runtime_error("Could not open file");
            std::string magic(5, '\0'); f.read(magic.data(), 5);
            if (magic != "VENC1") throw std::runtime_error("Invalid encrypted file format");
            std::vector<uint8_t> salt(16), nonce(12), tag(16);
            f.read(reinterpret_cast<char*>(salt.data()), (std::streamsize)salt.size());
            f.read(reinterpret_cast<char*>(nonce.data()), (std::streamsize)nonce.size());
            f.read(reinterpret_cast<char*>(tag.data()), (std::streamsize)tag.size());
            std::vector<uint8_t> ct((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            std::vector<uint8_t> key(32);
            PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt.data(), (int)salt.size(), 100000, EVP_sha256(), 32, key.data());
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, nullptr, nullptr);
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, (int)nonce.size(), nullptr);
            EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data());
            std::vector<uint8_t> pt(ct.size());
            int outlen = 0;
            EVP_DecryptUpdate(ctx, pt.data(), &outlen, ct.data(), (int)ct.size());
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, (int)tag.size(), tag.data());
            int ok = EVP_DecryptFinal_ex(ctx, nullptr, &outlen);
            EVP_CIPHER_CTX_free(ctx);
            if (ok != 1) throw std::runtime_error("Decrypt failed");
            auto sv = StringPool::intern(std::string(pt.begin(), pt.end()));
            return stringValue(sv.data());
        },
        "readFileEncAlgo"
    ));
#endif
    globals->define("securityStatus", std::make_shared<NativeFunction>(
        0,
        [globals](const std::vector<Value>&) -> Value {
            auto m = std::make_shared<ClawHashMap>();
            std::string sbox = (globals->sandbox() == Environment::SandboxMode::Strict ? std::string("strict") :
                globals->sandbox() == Environment::SandboxMode::Network ? std::string("network") : std::string("full"));
            m->set("sandbox", stringValue(StringPool::intern(sbox).data()));
            m->set("file.read", boolValue(globals->canFileRead()));
            m->set("file.write", boolValue(globals->canFileWrite()));
            m->set("file.delete", boolValue(globals->canFileDelete()));
            m->set("input", boolValue(globals->canInput()));
            m->set("output", boolValue(globals->canOutput()));
            m->set("network", boolValue(globals->canNetwork()));
            m->set("antiDebug", boolValue(globals->antiDebugEnforced()));
            m->set("dynamicCodeEnc", boolValue(globals->dynamicCodeEncryption()));
            m->set("cryptoPreferred", stringValue(StringPool::intern(globals->cryptoPreferred()).data()));
            m->set("ids.enabled", boolValue(gRuntimeFlags.idsEnabled));
            m->set("ids.stack.max", numberToValue((double)gRuntimeFlags.idsStackMax));
            m->set("ids.alloc.rate.max", numberToValue((double)gRuntimeFlags.idsAllocRateMax));
            return hashMapValue(m);
        },
        "securityStatus"
    ));
}
} 
