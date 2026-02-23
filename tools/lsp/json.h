#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>
#include <variant>

namespace claw::lsp {

struct Json;
using JsonObject = std::unordered_map<std::string, Json>;
using JsonArray = std::vector<Json>;

struct Json {
    enum class Type { Null, Bool, Number, String, Object, Array };
    Type type = Type::Null;
    bool b = false;
    double n = 0.0;
    std::string s;
    JsonObject o;
    JsonArray a;
    Json() = default;
    static Json null() { return Json(); }
    static Json boolean(bool v) { Json j; j.type = Type::Bool; j.b = v; return j; }
    static Json number(double v) { Json j; j.type = Type::Number; j.n = v; return j; }
    static Json string(std::string v) { Json j; j.type = Type::String; j.s = std::move(v); return j; }
    static Json object(JsonObject v) { Json j; j.type = Type::Object; j.o = std::move(v); return j; }
    static Json array(JsonArray v) { Json j; j.type = Type::Array; j.a = std::move(v); return j; }
};

class JsonParser {
public:
    explicit JsonParser(std::string_view input);
    Json parse();
private:
    std::string_view src_;
    size_t pos_ = 0;
    void skipWs();
    bool eof() const;
    char peek() const;
    char get();
    bool match(char c);
    Json parseValue();
    Json parseObject();
    Json parseArray();
    std::string parseString();
    double parseNumber();
    void expect(const char* kw);
};

std::string stringify(const Json& j);
std::optional<const Json*> getPath(const Json& j, const std::vector<std::string>& path);

} // namespace claw::lsp
