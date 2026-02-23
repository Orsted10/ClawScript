#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <sstream>

namespace claw {

class JSONTest : public ::testing::Test {
protected:
    std::string runExpression(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        Interpreter interpreter;
        // We need to capture the last value if it's an expression statement
        // but for simplicity, let's just use a global variable to store the result
        // or just execute and check the side effects/return value.
        // For JSON tests, we can just print the result and capture it.
        
        std::streambuf* oldCout = std::cout.rdbuf();
        std::stringstream capturedOutput;
        std::cout.rdbuf(capturedOutput.rdbuf());
        
        interpreter.execute(statements);
        
        std::cout.rdbuf(oldCout);
        return capturedOutput.str();
    }
};

TEST_F(JSONTest, BasicJSONDecode) {
    std::string code = "print jsonDecode(\"{\\\"name\\\": \\\"Alice\\\", \\\"age\\\": 30}\");";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Interpreter interpreter;
    
    std::stringstream ss;
    auto old_buf = std::cout.rdbuf(ss.rdbuf());
    interpreter.execute(statements);
    std::cout.rdbuf(old_buf);
    
    std::string output = ss.str();
    // VoltHashMap::toString returns {"key": value} where value is not quoted if it's a string
    EXPECT_TRUE(output.find("\"name\": Alice") != std::string::npos);
    EXPECT_TRUE(output.find("\"age\": 30") != std::string::npos);
}

TEST_F(JSONTest, NestedJSONDecode) {
    std::string code = "let data = jsonDecode(\"{\\\"items\\\": [1, 2, 3], \\\"meta\\\": {\\\"count\\\": 3}}\");\n"
                      "print data.items[1];\n"
                      "print data.meta.count;";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Interpreter interpreter;
    
    std::stringstream ss;
    auto old_buf = std::cout.rdbuf(ss.rdbuf());
    interpreter.execute(statements);
    std::cout.rdbuf(old_buf);
    
    EXPECT_EQ(ss.str(), "2\n3\n");
}

TEST_F(JSONTest, JSONEncode) {
    std::string code = "let map = {};\n"
                      "map.name = \"Bob\";\n"
                      "map.scores = [10, 20];\n"
                      "print jsonEncode(map);";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Interpreter interpreter;
    
    std::stringstream ss;
    auto old_buf = std::cout.rdbuf(ss.rdbuf());
    interpreter.execute(statements);
    std::cout.rdbuf(old_buf);
    
    std::string output = ss.str();
    EXPECT_TRUE(output.find("\"name\":\"Bob\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"scores\":[10,20]") != std::string::npos);
}

} // namespace claw
