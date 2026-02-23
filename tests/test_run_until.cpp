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

// ==================== BASIC RUN-UNTIL TESTS ====================

TEST(RunUntil, BasicLoop) {
    std::string code = R"(
        let i = 0;
        run {
            print i;
            i = i + 1;
        } until (i == 5);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n3\n4\n");
}

TEST(RunUntil, SingleIteration) {
    std::string code = R"(
        let x = 5;
        run {
            print x;
        } until (x == 5);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(RunUntil, MultipleIterations) {
    std::string code = R"(
        let count = 0;
        run {
            count++;
        } until (count >= 10);
        print count;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

// ==================== CONDITION TESTS ====================

TEST(RunUntil, ComplexCondition) {
    std::string code = R"(
        let a = 1;
        let b = 1;
        run {
            let temp = a + b;
            a = b;
            b = temp;
        } until (b > 50);
        print b;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "55\n");
}

TEST(RunUntil, LogicalCondition) {
    std::string code = R"(
        let x = 0;
        run {
            x++;
        } until (x >= 5 || x < 0);
        print x;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(RunUntil, NegatedCondition) {
    std::string code = R"(
        let flag = false;
        let i = 0;
        run {
            i++;
            if (i == 3) flag = true;
        } until (flag);
        print i;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

// ==================== BREAK AND CONTINUE ====================

TEST(RunUntil, WithBreak) {
    std::string code = R"(
        let i = 0;
        run {
            print i;
            i++;
            if (i >= 3) break;
        } until (i == 100);
        print "done";
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\ndone\n");
}

TEST(RunUntil, WithContinue) {
    std::string code = R"(
        let i = 0;
        run {
            i++;
            if (i % 2 == 0) continue;
            print i;
        } until (i >= 7);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n3\n5\n7\n");
}

TEST(RunUntil, BreakEarly) {
    std::string code = R"(
        let count = 0;
        run {
            count++;
            if (count == 1) break;
        } until (count >= 10);
        print count;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

// ==================== NESTED LOOPS ====================

TEST(RunUntil, NestedRunUntil) {
    std::string code = R"(
        let i = 0;
        run {
            let j = 0;
            run {
                print j;
                j++;
            } until (j >= 2);
            i++;
        } until (i >= 2);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n0\n1\n");
}

TEST(RunUntil, RunUntilInsideWhile) {
    std::string code = R"(
        let outer = 0;
        while (outer < 2) {
            let inner = 0;
            run {
                print inner;
                inner++;
            } until (inner >= 2);
            outer++;
        }
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n0\n1\n");
}

TEST(RunUntil, WhileInsideRunUntil) {
    std::string code = R"(
        let i = 0;
        run {
            let j = 0;
            while (j < 2) {
                print j;
                j++;
            }
            i++;
        } until (i >= 2);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n0\n1\n");
}

// ==================== BLOCK STATEMENTS ====================

TEST(RunUntil, WithBlockBody) {
    std::string code = R"(
        let i = 0;
        run {
            {
                let temp = i * 2;
                print temp;
            }
            i++;
        } until (i >= 3);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n2\n4\n");
}

TEST(RunUntil, SingleStatementBody) {
    std::string code = R"(
        let x = 0;
        run {
            x++;
        } until (x >= 5);
        print x;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

// ==================== VARIABLE SCOPE ====================

TEST(RunUntil, VariableScope) {
    std::string code = R"(
        let outer = 10;
        let i = 0;
        run {
            let inner = i;
            print inner;
            i++;
        } until (i >= 3);
        print outer;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n10\n");
}

// ==================== PRACTICAL EXAMPLES ====================

TEST(RunUntil, CountdownExample) {
    std::string code = R"(
        let countdown = 5;
        run {
            print countdown;
            countdown--;
        } until (countdown <= 0);
        print "Liftoff!";
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n4\n3\n2\n1\nLiftoff!\n");
}

TEST(RunUntil, MenuLoop) {
    std::string code = R"(
        let choice = 0;
        let iterations = 0;
        run {
            iterations++;
            choice = iterations;
        } until (choice == 3);
        print iterations;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(RunUntil, SumUntilThreshold) {
    std::string code = R"(
        let sum = 0;
        let n = 1;
        run {
            sum = sum + n;
            n++;
        } until (sum >= 50);
        print sum;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "55\n");
}

// ==================== EDGE CASES ====================

TEST(RunUntil, ImmediatelyTrue) {
    std::string code = R"(
        let x = 5;
        run {
            print "executed";
            x = 10;
        } until (true);
        print "done";
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "executed\ndone\n");
}

TEST(RunUntil, AlwaysFalse) {
    std::string code = R"(
        let i = 0;
        run {
            print i;
            i++;
            if (i >= 5) break;
        } until (false);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n3\n4\n");
}

TEST(RunUntil, WithArrays) {
    std::string code = R"(
        let arr = [];
        let i = 0;
        run {
            arr.push(i);
            i++;
        } until (i >= 5);
        print arr.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(RunUntil, WithFunctionCalls) {
    std::string code = R"(
        fn isEven(n) {
            return n % 2 == 0;
        }
        
        let n = 1;
        run {
            n++;
        } until (isEven(n) && n > 5);
        print n;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

// ==================== COMPARISON WITH WHILE ====================

TEST(RunUntil, DifferenceFromWhile) {
    // This test demonstrates that run-until executes at least once
    // even when condition is initially true
    std::string code = R"(
        let flag = true;
        let count = 0;
        
        // Run-until executes once even though flag is true
        run {
            count++;
        } until (flag);
        
        print count;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(RunUntil, CompareWithDoWhile) {
    // In traditional do-while: do { } while (condition is true)
    // In run-until: run { } until (condition is true)
    // They are opposites in logic!
    
    std::string code = R"(
        let x = 0;
        run {
            x++;
        } until (x == 5);
        
        let y = 0;
        while (y != 5) {
            y++;
        }
        
        print x;
        print y;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n5\n");
}
