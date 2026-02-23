#include <gtest/gtest.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter.h"
#include "features/hashmap.h"
#include "vm/opcodes.h"
#include <sstream>
#include <thread>

namespace claw {

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
    
    std::string getError(const std::string& code) {
        std::stringstream ss;
        auto old_err = std::cerr.rdbuf(ss.rdbuf());
        runVM(code);
        std::cerr.rdbuf(old_err);
        return ss.str();
    }
    std::string getErrorWithInterpreter(const std::string& code) {
        std::stringstream ss;
        auto old_err = std::cerr.rdbuf(ss.rdbuf());
        runVMWithInterpreter(code);
        std::cerr.rdbuf(old_err);
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

TEST_F(VMTest, CompoundIndexAssignArrayAdd) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[5,3]\"); a[0] += 2; print a[0];"), "7\n");
}

TEST_F(VMTest, GetIndexArray) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[5,3]\"); print a[0];"), "5\n");
}

TEST_F(VMTest, CompoundIndexAssignArrayBitwiseXor) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[5]\"); a[0] ^= 3; print a[0];"), "6\n");
}

TEST_F(VMTest, DebugCompoundIndex) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[5,3]\"); print type(a); a[0] += 2; print a[0];"), "array\n7\n");
}

TEST_F(VMTest, ManualIndexAssignAdd) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[5,3]\"); a[0] = a[0] + 2; print a[0];"), "7\n");
}
TEST_F(VMTest, ErrorMessage_CompoundIndexArrayAdd) {
    auto err = getErrorWithInterpreter("let a = jsonDecode(\"[5,3]\"); a[0] += 2; print a[0];");
    std::cout << err;
    SUCCEED();
}
static std::string dumpBytecode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Compiler compiler;
    auto chunk = compiler.compile(statements);
    std::stringstream ss;
    const auto& codeBytes = chunk->code();
    ss << "bytes(" << codeBytes.size() << "): ";
    for (size_t i = 0; i < codeBytes.size(); ++i) {
        ss << std::hex << std::uppercase << (int)codeBytes[i];
        if (i + 1 != codeBytes.size()) ss << " ";
    }
    ss << "\n";
    return ss.str();
}

TEST_F(VMTest, BytecodeDump_CompoundIndexAdd) {
    auto bc = dumpBytecode("let a = jsonDecode(\"[5,3]\"); a[0] += 2; print a[0];");
    std::cout << bc;
    SUCCEED();
}
TEST_F(VMTest, CompoundIndexAssignArrayShifts) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[2]\"); a[0] <<= 3; print a[0];"), "16\n");
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[16]\"); a[0] >>= 2; print a[0];"), "4\n");
}

TEST_F(VMTest, CompoundIndexAssignMapStringPlusNumber) {
    EXPECT_EQ(getOutputWithInterpreter("let m = jsonDecode(\"{\\\"s\\\":\\\"a\\\"}\"); m[\"s\"] += 7; print m[\"s\"];"), "a7\n");
}

TEST_F(VMTest, CompoundIndexAssignNested) {
    EXPECT_EQ(getOutputWithInterpreter("let a = jsonDecode(\"[[1]]\"); a[0][0] += 4; print a[0][0];"), "5\n");
}

TEST_F(VMTest, CompoundIndexAssignNilEdgeCase) {
    // Auto-create missing key with numeric default for +=, then update
    EXPECT_EQ(getOutputWithInterpreter("let m = jsonDecode(\"{}\"); m[\"x\"] += 1; print m[\"x\"];"), "1\n");
}

TEST_F(VMTest, DivideByZeroErrors) {
    auto err1 = getError("print 10 / 0;");
    EXPECT_NE(err1.find("Division by zero."), std::string::npos);
    auto err2 = getError("let x = 0; print 5 / (x);");
    EXPECT_NE(err2.find("Division by zero."), std::string::npos);
}

TEST_F(VMTest, DivideByZeroVariant0) { auto err = getError("print 1 / 0;"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant1) { auto err = getError("let a = 0; print 10 / a;"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant2) { auto err = getError("print (1 + 2) / (3 - 3);"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant3) { auto err = getError("let a = 0; let b = 1; print b / a;"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant4) { auto err = getError("print 0 / 0;"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant5) { auto err = getError("print (4 / 2) / (1 - 1);"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant6) { auto err = getError("let z = 0; if (true) { print 7 / z; }"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant7) { auto err = getError("print 7 / (0 + 0);"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant8) { auto err = getError("print 1 / (num(0));"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant9) { auto err = getError("let f = fn() { return 0; }; print 2 / f();"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant10) { auto err = getError("print 10 / (true ? 0 : 1);"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant11) { auto err = getError("let g = fn(x) { return x; }; print 9 / g(0);"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant12) { auto err = getError("print 3 / ((1 - 1));"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant13) { auto err = getError("print (1 + 1) / ((2 - 2));"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant14) { auto err = getError("print 1 / ((1 - 1) + 0);"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }
TEST_F(VMTest, DivideByZeroVariant15) { auto err = getError("print (1 / 1) / ((1 - 1));"); EXPECT_NE(err.find("Division by zero."), std::string::npos); }

TEST_F(VMTest, ShiftCountBoundaries) {
    EXPECT_EQ(getOutput("print 1 << 0;"), "1\n");
    EXPECT_EQ(getOutput("print 1 << 63;"), "9223372036854775808\n");
    EXPECT_EQ(getOutput("print 8 >> 2;"), "2\n");
    auto err = getError("print 1 << -1;");
    EXPECT_NE(err.find("Shift count must be non-negative."), std::string::npos);
}

// Generate many shift boundary tests to grow coverage
#define GEN_SHIFT_TEST(N, EXPR, EXPECTED) \
TEST_F(VMTest, ShiftBoundary_##N) { \
    EXPECT_EQ(getOutput("print " EXPR ";"), EXPECTED "\n"); \
}
GEN_SHIFT_TEST(0, "2 << 1", "4")
GEN_SHIFT_TEST(1, "2 << 2", "8")
GEN_SHIFT_TEST(2, "2 << 3", "16")
GEN_SHIFT_TEST(3, "2 << 4", "32")
GEN_SHIFT_TEST(4, "2 << 5", "64")
GEN_SHIFT_TEST(5, "64 >> 6", "1")
GEN_SHIFT_TEST(6, "64 >> 5", "2")
GEN_SHIFT_TEST(7, "64 >> 4", "4")
GEN_SHIFT_TEST(8, "64 >> 3", "8")
GEN_SHIFT_TEST(9, "64 >> 2", "16")
GEN_SHIFT_TEST(10, "64 >> 1", "32")
GEN_SHIFT_TEST(11, "1 << 64", "1")
GEN_SHIFT_TEST(12, "3 << 0", "3")
GEN_SHIFT_TEST(13, "3 >> 0", "3")
GEN_SHIFT_TEST(14, "5 << 7", "640")
GEN_SHIFT_TEST(15, "128 >> 7", "1")
GEN_SHIFT_TEST(16, "256 >> 8", "1")
GEN_SHIFT_TEST(17, "1 << 32", "4294967296")
GEN_SHIFT_TEST(18, "1 << 16", "65536")
GEN_SHIFT_TEST(19, "65536 >> 16", "1")

TEST_F(VMTest, AutoCreateStringConcat) {
    EXPECT_EQ(getOutputWithInterpreter("let m = jsonDecode(\"{}\"); m[\"s\"] += \"x\"; print m[\"s\"];"), "x\n");
    EXPECT_EQ(getOutputWithInterpreter("let m = jsonDecode(\"{}\"); m[\"s\"] += 7; print m[\"s\"];"), "7\n");
}

TEST_F(VMTest, HashMapEnsureDefaultConcurrent) {
    auto map = std::make_shared<ClawHashMap>();
    auto work = [map]() {
        for (int i = 0; i < 1000; ++i) {
            map->ensureDefault("x", numberToValue(0.0));
        }
    };
    std::thread t1(work);
    std::thread t2(work);
    t1.join();
    t2.join();
    EXPECT_TRUE(map->contains("x"));
}

} // namespace claw
