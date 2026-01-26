#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <sstream>

using namespace volt;

namespace {

// Helper to capture print output
class PrintCapture {
public:
    PrintCapture() : oldBuf_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(oldBuf_); }
    std::string getOutput() const { return buffer_.str(); }
private:
    std::stringstream buffer_;
    std::streambuf* oldBuf_;
};

// Helper function to run code and capture output
std::string runCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) {
        return "PARSE_ERROR";
    }
    
    PrintCapture capture;
    Interpreter interpreter;
    
    try {
        interpreter.execute(statements);
    } catch (const std::exception& e) {
        return std::string("RUNTIME_ERROR: ") + e.what();
    }
    
    return capture.getOutput();
}

} // anonymous namespace

// ==================== NEW HASH MAP FUNCTIONALITY TESTS ====================

TEST(ImprovedHashMap, HasFunction) {
    std::string code = R"(
        let map = {"exists": "yes", "also_exists": "maybe"};
        print has(map, "exists");
        print has(map, "also_exists");
        print has(map, "does_not_exist");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\ntrue\nfalse\n");
}

TEST(ImprovedHashMap, RemoveFunction) {
    std::string code = R"(
        let map = {"a": 1, "b": 2, "c": 3};
        print remove(map, "b");
        print has(map, "b");
        print map["a"];
        print map["c"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\nfalse\n1\n3\n");
}

TEST(ImprovedHashMap, HasMethod) {
    std::string code = R"(
        let map = {"key1": "value1", "key2": "value2"};
        print map.has("key1");
        print map.has("nonexistent");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\nfalse\n");
}

TEST(ImprovedHashMap, RemoveMethod) {
    std::string code = R"(
        let map = {"a": 10, "b": 20, "c": 30};
        print map.remove("b");
        print map.has("b");
        print map["a"];
        print map["c"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\nfalse\n10\n30\n");
}

TEST(ImprovedHashMap, MergeFunctionality) {
    std::string code = R"(
        let map1 = {"a": 1, "b": 2};
        let map2 = {"c": 3, "d": 4};
        print map1.size;
        print map2.size;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n2\n");
}

TEST(ImprovedHashMap, MixedOperations) {
    std::string code = R"(
        let config = {
            "debug": true,
            "port": 8080,
            "host": "localhost"
        };
        
        print has(config, "debug");
        config["new_setting"] = "enabled";
        print has(config, "new_setting");
        print remove(config, "port");
        print has(config, "port");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\ntrue\ntrue\nfalse\n");
}