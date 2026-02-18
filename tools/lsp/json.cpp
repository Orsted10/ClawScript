#include "json.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace volt::lsp {

JsonParser::JsonParser(std::string_view input) : src_(input) {}

void JsonParser::skipWs() {
    while (pos_ < src_.size()) {
        char c = src_[pos_];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { pos_++; continue; }
        break;
    }
}

bool JsonParser::eof() const { return pos_ >= src_.size(); }
char JsonParser::peek() const { return eof() ? '\0' : src_[pos_]; }
char JsonParser::get() { return eof() ? '\0' : src_[pos_++]; }
bool JsonParser::match(char c) { if (peek() == c) { pos_++; return true; } return false; }

void JsonParser::expect(const char* kw) {
    for (const char* p = kw; *p; ++p) {
        if (eof() || get() != *p) throw std::runtime_error("Invalid JSON literal");
    }
}

std::string JsonParser::parseString() {
    if (!match('"')) throw std::runtime_error("Expected string");
    std::string out;
    while (!eof()) {
        char c = get();
        if (c == '"') break;
        if (c == '\\') {
            char e = get();
            switch (e) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default: out.push_back(e); break;
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

double JsonParser::parseNumber() {
    size_t start = pos_;
    if (peek() == '-') get();
    while (std::isdigit(peek())) get();
    if (peek() == '.') {
        get();
        while (std::isdigit(peek())) get();
    }
    if (peek() == 'e' || peek() == 'E') {
        get();
        if (peek() == '+' || peek() == '-') get();
        while (std::isdigit(peek())) get();
    }
    std::string_view sv = src_.substr(start, pos_ - start);
    return std::stod(std::string(sv));
}

Json JsonParser::parseArray() {
    if (!match('[')) throw std::runtime_error("Expected '['");
    JsonArray arr;
    skipWs();
    if (match(']')) return Json::array(std::move(arr));
    while (true) {
        skipWs();
        arr.push_back(parseValue());
        skipWs();
        if (match(']')) break;
        if (!match(',')) throw std::runtime_error("Expected ',' in array");
    }
    return Json::array(std::move(arr));
}

Json JsonParser::parseObject() {
    if (!match('{')) throw std::runtime_error("Expected '{'");
    JsonObject obj;
    skipWs();
    if (match('}')) return Json::object(std::move(obj));
    while (true) {
        skipWs();
        std::string key = parseString();
        skipWs();
        if (!match(':')) throw std::runtime_error("Expected ':' in object");
        skipWs();
        Json val = parseValue();
        obj.emplace(std::move(key), std::move(val));
        skipWs();
        if (match('}')) break;
        if (!match(',')) throw std::runtime_error("Expected ',' in object");
    }
    return Json::object(std::move(obj));
}

Json JsonParser::parseValue() {
    skipWs();
    char c = peek();
    if (c == '"') return Json::string(parseString());
    if (c == '{') return parseObject();
    if (c == '[') return parseArray();
    if (c == 't') { expect("true"); return Json::boolean(true); }
    if (c == 'f') { expect("false"); return Json::boolean(false); }
    if (c == 'n') { expect("null"); return Json::null(); }
    return Json::number(parseNumber());
}

Json JsonParser::parse() {
    skipWs();
    return parseValue();
}

static void stringifyImpl(const Json& j, std::ostringstream& oss) {
    switch (j.type) {
        case Json::Type::Null: oss << "null"; break;
        case Json::Type::Bool: oss << (j.b ? "true" : "false"); break;
        case Json::Type::Number: {
            oss << std::setprecision(17) << j.n;
            break;
        }
        case Json::Type::String: {
            oss << "\"";
            for (char c : j.s) {
                switch (c) {
                    case '"': oss << "\\\""; break;
                    case '\\': oss << "\\\\"; break;
                    case '\n': oss << "\\n"; break;
                    case '\r': oss << "\\r"; break;
                    case '\t': oss << "\\t"; break;
                    default: oss << c; break;
                }
            }
            oss << "\"";
            break;
        }
        case Json::Type::Object: {
            oss << "{";
            bool first = true;
            for (const auto& [k, v] : j.o) {
                if (!first) oss << ",";
                first = false;
                oss << "\"" << k << "\":";
                stringifyImpl(v, oss);
            }
            oss << "}";
            break;
        }
        case Json::Type::Array: {
            oss << "[";
            bool first = true;
            for (const auto& v : j.a) {
                if (!first) oss << ",";
                first = false;
                stringifyImpl(v, oss);
            }
            oss << "]";
            break;
        }
    }
}

std::string stringify(const Json& j) {
    std::ostringstream oss;
    stringifyImpl(j, oss);
    return oss.str();
}

std::optional<const Json*> getPath(const Json& j, const std::vector<std::string>& path) {
    const Json* cur = &j;
    for (const auto& key : path) {
        if (cur->type != Json::Type::Object) return std::nullopt;
        auto it = cur->o.find(key);
        if (it == cur->o.end()) return std::nullopt;
        cur = &it->second;
    }
    return cur;
}

} // namespace volt::lsp

