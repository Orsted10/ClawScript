#include "interpreter/natives/native_json.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include "features/class.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

namespace volt {

class JSONParser {
public:
    explicit JSONParser(const std::string& source) : source_(source), pos_(0) {}

    Value parse() {
        skipWhitespace();
        if (pos_ >= source_.length()) return nilValue();
        
        Value val = parseValue();
        skipWhitespace();
        return val;
    }

private:
    Value parseValue() {
        skipWhitespace();
        if (pos_ >= source_.length()) throw std::runtime_error("Unexpected EOF");

        char c = source_[pos_];
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (isdigit(c) || c == '-') return parseNumber();

        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    Value parseObject() {
        consume('{');
        auto map = std::make_shared<VoltHashMap>();
        
        skipWhitespace();
        if (peek() == '}') {
            consume('}');
            return hashMapValue(map);
        }

        while (true) {
            skipWhitespace();
            std::string key = asString(parseString());
            skipWhitespace();
            consume(':');
            Value val = parseValue();
            map->set(key, val);

            skipWhitespace();
            if (peek() == '}') {
                consume('}');
                break;
            }
            consume(',');
        }
        return hashMapValue(map);
    }

    Value parseArray() {
        consume('[');
        auto arr = std::make_shared<VoltArray>();

        skipWhitespace();
        if (peek() == ']') {
            consume(']');
            return arrayValue(arr);
        }

        while (true) {
            arr->push(parseValue());
            skipWhitespace();
            if (peek() == ']') {
                consume(']');
                break;
            }
            consume(',');
        }
        return arrayValue(arr);
    }

    Value parseString() {
        consume('"');
        std::string result;
        while (pos_ < source_.length() && source_[pos_] != '"') {
            if (source_[pos_] == '\\') {
                pos_++;
                if (pos_ >= source_.length()) throw std::runtime_error("Unterminated escape sequence");
                switch (source_[pos_]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        // Very basic unicode - just skip for now or handle properly
                        if (pos_ + 4 >= source_.length()) throw std::runtime_error("Invalid unicode escape");
                        pos_ += 4; // Skip the hex digits
                        result += '?'; // Placeholder
                        break;
                    }
                    default: result += source_[pos_]; break;
                }
            } else {
                result += source_[pos_];
            }
            pos_++;
        }
        consume('"');
        auto sv = StringPool::intern(result);
        return stringValue(sv.data());
    }

    Value parseNumber() {
        size_t start = pos_;
        if (source_[pos_] == '-') pos_++;
        while (pos_ < source_.length() && (isdigit(source_[pos_]) || source_[pos_] == '.')) pos_++;
        
        std::string numStr = source_.substr(start, pos_ - start);
        return numberToValue(std::stod(numStr));
    }

    Value parseBool() {
        if (source_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return boolValue(true);
        }
        if (source_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return boolValue(false);
        }
        throw std::runtime_error("Invalid boolean");
    }

    Value parseNull() {
        if (source_.substr(pos_, 4) == "null") {
            pos_ += 4;
            return nilValue();
        }
        throw std::runtime_error("Invalid null");
    }

    void skipWhitespace() {
        while (pos_ < source_.length() && isspace(source_[pos_])) pos_++;
    }

    char peek() {
        if (pos_ >= source_.length()) return '\0';
        return source_[pos_];
    }

    void consume(char expected) {
        skipWhitespace();
        if (pos_ >= source_.length() || source_[pos_] != expected) {
            throw std::runtime_error(std::string("Expected '") + expected + "' but got '" + (pos_ < source_.length() ? std::string(1, source_[pos_]) : "EOF") + "'");
        }
        pos_++;
    }

    std::string source_;
    size_t pos_;
};

class JSONEncoder {
public:
    static std::string encode(const Value& value) {
        std::ostringstream oss;
        encodeValue(value, oss);
        return oss.str();
    }

private:
    static void encodeValue(const Value& value, std::ostringstream& oss) {
        if (isNil(value)) {
            oss << "null";
        } else if (isBool(value)) {
            oss << (asBool(value) ? "true" : "false");
        } else if (isNumber(value)) {
            double num = asNumber(value);
            if (num == static_cast<long long>(num)) {
                oss << static_cast<long long>(num);
            } else {
                oss << std::fixed << std::setprecision(6) << num;
                std::string str = oss.str();
                oss.str("");
                oss.clear();
                size_t dotPos = str.find('.');
                if (dotPos != std::string::npos) {
                    size_t lastNonZero = str.find_last_not_of('0');
                    if (lastNonZero == dotPos) {
                        str = str.substr(0, dotPos);
                    } else if (lastNonZero != std::string::npos) {
                        str = str.substr(0, lastNonZero + 1);
                    }
                }
                oss << str;
            }
        } else if (isString(value)) {
            oss << "\"" << escapeString(asString(value)) << "\"";
        } else if (isArray(value)) {
            auto array = asArray(value);
            oss << "[";
            const auto& elements = array->elements();
            for (size_t i = 0; i < elements.size(); i++) {
                if (i > 0) oss << ",";
                encodeValue(elements[i], oss);
            }
            oss << "]";
        } else if (isHashMap(value)) {
            auto map = asHashMap(value);
            oss << "{";
            auto keys = map->getKeys();
            for (size_t i = 0; i < keys.size(); i++) {
                if (i > 0) oss << ",";
                oss << "\"" << escapeString(keys[i]) << "\":";
                encodeValue(map->get(keys[i]), oss);
            }
            oss << "}";
        } else if (isInstance(value)) {
            auto instance = asInstance(value);
            oss << "{\"_class\":\"" << instance->getClass()->getName() << "\"}";
        } else {
            oss << "null";
        }
    }

    static std::string escapeString(const std::string& str) {
        std::ostringstream oss;
        for (char c : str) {
            switch (c) {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 32) {
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        oss << c;
                    }
                    break;
            }
        }
        return oss.str();
    }
};

void registerNativeJSON(const std::shared_ptr<Environment>& globals) {
    globals->define("jsonEncode", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto sv = StringPool::intern(JSONEncoder::encode(args[0]));
            return stringValue(sv.data());
        },
        "jsonEncode"
    ));

    globals->define("jsonDecode", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("jsonDecode requires a string");
            JSONParser parser(asString(args[0]));
            return parser.parse();
        },
        "jsonDecode"
    ));
}

} // namespace volt
