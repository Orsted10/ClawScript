#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <string>

namespace {

struct ErrorInfo {
    bool hadError;
    int line;
    int column;
    std::string message;
};

ErrorInfo getRuntimeError(const std::string& source) {
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) return {false, 0, 0, "PARSE_ERROR"};
    
    volt::Interpreter interpreter;
    try {
        interpreter.execute(statements);
    } catch (const volt::RuntimeError& e) {
        return {true, e.token.line, e.token.column, e.what()};
    } catch (...) {
        return {false, 0, 0, "OTHER_ERROR"};
    }
    
    return {false, 0, 0, "NO_ERROR"};
}

} // anonymous namespace

TEST(ErrorReporting, DivisionByZero) {
    ErrorInfo info = getRuntimeError("print 10 / 0;");
    ASSERT_TRUE(info.hadError);
    EXPECT_EQ(info.line, 1);
    // '/' is at column 10 (1-based: p r i n t _ 1 0 _ /)
    // "print 10 / 0;"
    //  1234567890
    EXPECT_EQ(info.column, 10);
    EXPECT_TRUE(info.message.find("zero") != std::string::npos);
}

TEST(ErrorReporting, UndefinedVariable) {
    ErrorInfo info = getRuntimeError("print x;");
    ASSERT_TRUE(info.hadError);
    EXPECT_EQ(info.line, 1);
    EXPECT_EQ(info.column, 7); // 'x' starts at col 7
}

TEST(ErrorReporting, ArrayIndexOutOfBounds) {
    ErrorInfo info = getRuntimeError("let a = [1, 2];\nprint a[5];");
    ASSERT_TRUE(info.hadError);
    EXPECT_EQ(info.line, 2);
    EXPECT_EQ(info.column, 8); // '[' is at col 8 in line 2
}

TEST(ErrorReporting, MultiLineError) {
    ErrorInfo info = getRuntimeError(
        "let x = 10;\n"
        "let y = 20;\n"
        "let z = x + y;\n"
        "print z / (x - 10);" // Division by zero on line 4
    );
    ASSERT_TRUE(info.hadError);
    EXPECT_EQ(info.line, 4);
    EXPECT_EQ(info.column, 9); // '/' is at col 9
}

TEST(ErrorReporting, InvalidMemberAccess) {
    ErrorInfo info = getRuntimeError("let n = 42;\nprint n.length;");
    ASSERT_TRUE(info.hadError);
    EXPECT_EQ(info.line, 2);
    EXPECT_EQ(info.column, 9); // 'length' starts at col 9
}
