#include "interpreter/natives/native_string.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include "features/array.h"
#include "features/hashmap.h"
#include "features/string_pool.h"
#include <string>
#include <sstream>
#include <cctype>

namespace claw {

void registerNativeString(const std::shared_ptr<Environment>& globals) {
    globals->define("len", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isString(args[0])) {
                return numberToValue(static_cast<double>(asString(args[0]).length()));
            }
            if (isArray(args[0])) {
                return numberToValue(static_cast<double>(asArray(args[0])->length()));
            }
            if (isHashMap(args[0])) {
                return numberToValue(static_cast<double>(asHashMap(args[0])->size()));
            }
            throw std::runtime_error("len() requires a string, array, or hash map argument");
        },
        "len"
    ));

    globals->define("str", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto sv = StringPool::intern(valueToString(args[0]));
            return stringValue(sv.data());
        },
        "str"
    ));

    globals->define("toUpper", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("toUpper() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            auto sv = StringPool::intern(s);
            return stringValue(sv.data());
        },
        "toUpper"
    ));

    globals->define("toLower", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("toLower() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            auto sv = StringPool::intern(s);
            return stringValue(sv.data());
        },
        "toLower"
    ));

    globals->define("substr", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("substr() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("substr() requires a number as start position");
            if (!isNumber(args[2])) throw std::runtime_error("substr() requires a number as length");
            std::string s = asString(args[0]);
            int start = static_cast<int>(asNumber(args[1]));
            int length = static_cast<int>(asNumber(args[2]));
            if (start < 0) start = 0;
            if (start >= static_cast<int>(s.length())) {
                auto sv = StringPool::intern(std::string(""));
                return stringValue(sv.data());
            }
            if (length < 0) length = 0;
            if (start + length > static_cast<int>(s.length())) {
                length = static_cast<int>(s.length()) - start;
            }
            auto sv = StringPool::intern(s.substr(start, length));
            return stringValue(sv.data());
        },
        "substr"
    ));

    globals->define("indexOf", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("indexOf() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("indexOf() requires a string as second argument");
            std::string s = asString(args[0]);
            std::string sub = asString(args[1]);
            size_t pos = s.find(sub);
            if (pos == std::string::npos) {
                return numberToValue(-1.0);
            }
            return numberToValue(static_cast<double>(pos));
        },
        "indexOf"
    ));

    globals->define("trim", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("trim() requires a string");
            std::string s = asString(args[0]);
            size_t start = 0;
            while (start < s.length() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
            size_t end = s.length();
            while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
            auto sv = StringPool::intern(s.substr(start, end - start));
            return stringValue(sv.data());
        },
        "trim"
    ));

    globals->define("split", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("split() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("split() requires a delimiter string");
            std::string s = asString(args[0]);
            std::string delimiter = asString(args[1]);
            auto result = std::make_shared<ClawArray>();
            if (delimiter.empty()) {
                for (char c : s) {
                    auto sv = StringPool::intern(std::string(1, c));
                    result->push(stringValue(sv.data()));
                }
                return arrayValue(result);
            }
            size_t pos = 0;
            while (true) {
                size_t next = s.find(delimiter, pos);
                if (next == std::string::npos) {
                    auto sv = StringPool::intern(s.substr(pos));
                    result->push(stringValue(sv.data()));
                    break;
                }
                auto sv = StringPool::intern(s.substr(pos, next - pos));
                result->push(stringValue(sv.data()));
                pos = next + delimiter.length();
            }
            return arrayValue(result);
        },
        "split"
    ));

    globals->define("replace", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("replace() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("replace() requires a string search pattern");
            if (!isString(args[2])) throw std::runtime_error("replace() requires a string replacement");
            std::string s = asString(args[0]);
            std::string search = asString(args[1]);
            std::string replacement = asString(args[2]);
            if (search.empty()) {
                auto sv0 = StringPool::intern(s);
                return stringValue(sv0.data());
            }
            std::string result = s;
            size_t pos = 0;
            while ((pos = result.find(search, pos)) != std::string::npos) {
                result.replace(pos, search.length(), replacement);
                pos += replacement.length();
                if (replacement.empty()) pos++;
            }
            auto sv = StringPool::intern(result);
            return stringValue(sv.data());
        },
        "replace"
    ));

    globals->define("startsWith", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("startsWith() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("startsWith() requires a string prefix");
            std::string s = asString(args[0]);
            std::string prefix = asString(args[1]);
            if (prefix.length() > s.length()) return boolValue(false);
            return boolValue(s.compare(0, prefix.length(), prefix) == 0);
        },
        "startsWith"
    ));

    globals->define("endsWith", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("endsWith() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("endsWith() requires a string suffix");
            std::string s = asString(args[0]);
            std::string suffix = asString(args[1]);
            if (suffix.length() > s.length()) return boolValue(false);
            return boolValue(s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
        },
        "endsWith"
    ));

    globals->define("repeat", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("repeat() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("repeat() requires a count number");
            std::string s = asString(args[0]);
            int count = static_cast<int>(asNumber(args[1]));
            if (count < 0) count = 0;
            std::ostringstream oss;
            for (int i = 0; i < count; i++) {
                oss << s;
            }
            auto sv = StringPool::intern(oss.str());
            return stringValue(sv.data());
        },
        "repeat"
    ));
}

} // namespace claw
