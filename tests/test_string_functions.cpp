#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <sstream>

using namespace claw;

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

// ==================== STRING FUNCTION TESTS ====================

TEST(StringFunctions, UpperFunction) {
    std::string code = R"(
        print toUpper("hello world");
        print toUpper("MiXeD CaSe");
        print toUpper("123 ABC");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "HELLO WORLD\nMIXED CASE\n123 ABC\n");
}

TEST(StringFunctions, LowerFunction) {
    std::string code = R"(
        print toLower("HELLO WORLD");
        print toLower("MiXeD CaSe");
        print toLower("123 ABC");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello world\nmixed case\n123 abc\n");
}

TEST(StringFunctions, SubstrFunction) {
    std::string code = R"(
        print substr("hello world", 0, 5);   // "hello"
        print substr("hello world", 6, 5);   // "world"
        print substr("hello world", 2, 3);   // "llo"
        print substr("hello", 10, 5);        // ""
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello\nworld\nllo\n\n");
}

TEST(StringFunctions, IndexOfFunction) {
    std::string code = R"(
        print indexOf("hello world", "world");  // 6
        print indexOf("hello world", "hello");  // 0
        print indexOf("hello world", "xyz");    // -1
        print indexOf("hello world", "o");      // 4
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n0\n-1\n4\n");
}

TEST(StringFunctions, LenFunctionEnhancement) {
    std::string code = R"(
        let str = "hello";
        let arr = [1, 2, 3];
        let map = {"a": 1, "b": 2, "c": 3};
        
        print len(str);
        print len(arr);
        print len(map);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n3\n3\n");
}

TEST(StringFunctions, StringChaining) {
    std::string code = R"(
        let text = "  Hello World  ";
        print toUpper(toLower(text));  // Should be "  HELLO WORLD  "
        print len(substr(text, 2, 5));  // length of " Hel"
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "  HELLO WORLD  \n5\n");
}

TEST(StringFunctions, EdgeCases) {
    std::string code = R"(
        print toUpper("");      // Empty string
        print toLower("");      // Empty string
        print substr("", 0, 5);  // Empty string
        print indexOf("", "x");  // Empty string
        print substr("test", 100, 5);  // Out of bounds
        print substr("test", 2, 100);  // Length out of bounds
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "\n\n\n-1\n\nst\n");
}
