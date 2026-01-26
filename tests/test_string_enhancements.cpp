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

// ==================== STRING ENHANCEMENT TESTS ====================

TEST(StringEnhancements, TrimFunction) {
    std::string code = R"(
        print trim("  hello  ");
        print trim("\t\n  world  \r\n");
        print trim("   ");
        print trim("");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello\nworld\n\n\n");
}

TEST(StringEnhancements, SplitFunction) {
    std::string code = R"(
        let parts = split("a,b,c", ",");
        print parts[0];
        print parts[1];
        print parts[2];
        print parts.length;
        
        let chars = split("hello", "");
        print chars[0];
        print chars[1];
        print chars.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "a\nb\nc\n3\nh\ne\n5\n");
}

TEST(StringEnhancements, ReplaceFunction) {
    std::string code = R"(
        print replace("hello world", "world", "there");
        print replace("aaa", "a", "b");
        print replace("test", "xyz", "replacement");
        print replace("banana", "ana", "ANA");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello there\nbbb\ntest\nbANAna\n");
}

TEST(StringEnhancements, StartsWithFunction) {
    std::string code = R"(
        print startsWith("hello world", "hello");
        print startsWith("hello world", "world");
        print startsWith("test", "");
        print startsWith("", "test");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\nfalse\ntrue\nfalse\n");
}

TEST(StringEnhancements, EndsWithFunction) {
    std::string code = R"(
        print endsWith("hello world", "world");
        print endsWith("hello world", "hello");
        print endsWith("test", "");
        print endsWith("", "test");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\nfalse\ntrue\nfalse\n");
}

TEST(StringEnhancements, CombinedOperations) {
    std::string code = R"(
        let text = "  Hello, World!  ";
        let trimmed = trim(text);
        let parts = split(trimmed, ",");
        let greeting = trim(parts[0]);
        let subject = trim(replace(parts[1], "!", ""));
        
        print greeting;
        print subject;
        print startsWith(greeting, "Hello");
        print endsWith(subject, "World");
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "Hello\nWorld\ntrue\ntrue\n");
}