#include <gtest/gtest.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter.h"
#include <sstream>

namespace volt {

class VMTest : public ::testing::Test {
protected:
    InterpretResult runVM(const std::string& code) {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        Compiler compiler;
        auto chunk = compiler.compile(statements);
        
        VM vm;
        return vm.interpret(*chunk);
    }

    InterpretResult runVMWithInterpreter(const std::string& code) {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        Interpreter interpreter;
        Compiler compiler;
        auto chunk = compiler.compile(statements);
        
        VM vm(interpreter);
        return vm.interpret(*chunk);
    }

    std::string getOutput(const std::string& code) {
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        runVM(code);
        std::cout.rdbuf(old_buf);
        return ss.str();
    }

    std::string getOutputWithInterpreter(const std::string& code) {
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        runVMWithInterpreter(code);
        std::cout.rdbuf(old_buf);
        return ss.str();
    }
};

TEST_F(VMTest, BasicArithmetic) {
    EXPECT_EQ(getOutput("print 1 + 2;"), "3\n");
    EXPECT_EQ(getOutput("print 10 - 4;"), "6\n");
    EXPECT_EQ(getOutput("print 3 * 4;"), "12\n");
    EXPECT_EQ(getOutput("print 20 / 5;"), "4\n");
}

TEST_F(VMTest, UnaryOps) {
    EXPECT_EQ(getOutput("print -5;"), "-5\n");
}

TEST_F(VMTest, StringConcatenation) {
    EXPECT_EQ(getOutput("print \"hello \" + \"world\";"), "hello world\n");
}

TEST_F(VMTest, GlobalVariables) {
    EXPECT_EQ(getOutput("let x = 10; print x;"), "10\n");
    EXPECT_EQ(getOutput("let a = 5; let b = 7; print a + b;"), "12\n");
}

TEST_F(VMTest, ComplexExpressions) {
    EXPECT_EQ(getOutput("print (1 + 2) * (3 + 4);"), "21\n");
}

TEST_F(VMTest, LocalVariables) {
    EXPECT_EQ(getOutput("{ let x = 10; print x; }"), "10\n");
    EXPECT_EQ(getOutput("{ let x = 10; { let x = 20; print x; } print x; }"), "20\n10\n");
}

TEST_F(VMTest, IfStatements) {
    EXPECT_EQ(getOutput("if (true) { print 1; } else { print 2; }"), "1\n");
    EXPECT_EQ(getOutput("if (false) { print 1; } else { print 2; }"), "2\n");
    EXPECT_EQ(getOutput("if (true) { print 1; }"), "1\n");
    EXPECT_EQ(getOutput("if (false) { print 1; }"), "");
}

TEST_F(VMTest, WhileLoops) {
    EXPECT_EQ(getOutput("let i = 0; while (i < 3) { print i; i = i + 1; }"), "0\n1\n2\n");
}

TEST_F(VMTest, NativeFunctionCallThroughVM) {
    EXPECT_EQ(getOutputWithInterpreter("print num(41) + 1;"), "42\n");
}

} // namespace volt
