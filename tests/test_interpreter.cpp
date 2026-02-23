#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <sstream>
#include <iostream>

// Helper to capture print output
class PrintCapture {
public:
    PrintCapture() : old(std::cout.rdbuf(buffer.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(old); }
    std::string get() { return buffer.str(); }
private:
    std::stringstream buffer;
    std::streambuf* old;
};

// Helper function to run code
std::string runCode(const std::string& source) {
    PrintCapture capture;
    
    claw::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    claw::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) return "PARSE_ERROR";
    
    claw::Interpreter interpreter;
    try {
        interpreter.execute(statements);
        return capture.get();
    } catch (...) {
        return "RUNTIME_ERROR";
    }
}

// ========================================
// PRINT STATEMENT TESTS
// ========================================

TEST(Interpreter, PrintNumber) {
    std::string output = runCode("print 42;");
    EXPECT_EQ(output, "42\n");
}

TEST(Interpreter, PrintString) {
    std::string output = runCode("print \"hello\";");
    EXPECT_EQ(output, "hello\n");
}

TEST(Interpreter, PrintBoolean) {
    EXPECT_EQ(runCode("print true;"), "true\n");
    EXPECT_EQ(runCode("print false;"), "false\n");
}

TEST(Interpreter, PrintNil) {
    std::string output = runCode("print nil;");
    EXPECT_EQ(output, "nil\n");
}

TEST(Interpreter, PrintExpression) {
    EXPECT_EQ(runCode("print 1 + 2;"), "3\n");
    EXPECT_EQ(runCode("print 10 * 5;"), "50\n");
    EXPECT_EQ(runCode("print \"hello\" + \" world\";"), "hello world\n");
}

TEST(Interpreter, MultiplePrints) {
    std::string output = runCode("print 1; print 2; print 3;");
    EXPECT_EQ(output, "1\n2\n3\n");
}

// ========================================
// VARIABLE DECLARATION TESTS
// ========================================

TEST(Interpreter, LetDeclaration) {
    std::string output = runCode("let x = 10; print x;");
    EXPECT_EQ(output, "10\n");
}

TEST(Interpreter, LetWithoutInitializer) {
    std::string output = runCode("let x; print x;");
    EXPECT_EQ(output, "nil\n");
}

TEST(Interpreter, MultipleVariables) {
    std::string output = runCode("let x = 5; let y = 10; print x + y;");
    EXPECT_EQ(output, "15\n");
}

TEST(Interpreter, VariableReassignment) {
    std::string output = runCode("let x = 5; x = 10; print x;");
    EXPECT_EQ(output, "10\n");
}

TEST(Interpreter, VariableExpressions) {
    std::string output = runCode("let x = 5; let y = x * 2; print y;");
    EXPECT_EQ(output, "10\n");
}

TEST(Interpreter, StringVariables) {
    std::string output = runCode("let name = \"Alice\"; print name;");
    EXPECT_EQ(output, "Alice\n");
}

// ========================================
// BLOCK SCOPE TESTS
// ========================================

TEST(Interpreter, SimpleBlock) {
    std::string output = runCode("{ print 42; }");
    EXPECT_EQ(output, "42\n");
}

TEST(Interpreter, BlockScope) {
    std::string output = runCode(
        "let x = 10;"
        "{"
        "  let x = 20;"
        "  print x;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output, "20\n10\n");
}

TEST(Interpreter, NestedBlocks) {
    std::string output = runCode(
        "let x = 1;"
        "{"
        "  let x = 2;"
        "  {"
        "    let x = 3;"
        "    print x;"
        "  }"
        "  print x;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output, "3\n2\n1\n");
}

TEST(Interpreter, BlockWithMultipleStatements) {
    std::string output = runCode(
        "{"
        "  let a = 5;"
        "  let b = 10;"
        "  print a + b;"
        "}"
    );
    EXPECT_EQ(output, "15\n");
}

TEST(Interpreter, VariableNotVisibleOutsideBlock) {
    std::string output = runCode(
        "{"
        "  let x = 10;"
        "}"
        "print x;"  // x is not defined here
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Interpreter, AccessOuterScope) {
    std::string output = runCode(
        "let x = 10;"
        "{"
        "  print x;"
        "  x = 20;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output, "10\n20\n");
}

// ========================================
// IF STATEMENT TESTS
// ========================================

TEST(Interpreter, IfTrue) {
    std::string output = runCode("if (true) print \"yes\";");
    EXPECT_EQ(output, "yes\n");
}

TEST(Interpreter, IfFalse) {
    std::string output = runCode("if (false) print \"yes\";");
    EXPECT_EQ(output, "");
}

TEST(Interpreter, IfElse) {
    std::string output = runCode("if (false) print \"yes\"; else print \"no\";");
    EXPECT_EQ(output, "no\n");
}

TEST(Interpreter, IfWithComparison) {
    EXPECT_EQ(runCode("if (5 > 3) print \"greater\";"), "greater\n");
    EXPECT_EQ(runCode("if (5 < 3) print \"less\";"), "");
}

TEST(Interpreter, IfWithVariables) {
    std::string output = runCode(
        "let x = 10;"
        "if (x > 5) print \"big\";"
    );
    EXPECT_EQ(output, "big\n");
}

TEST(Interpreter, IfElseChain) {
    std::string output = runCode(
        "let x = 5;"
        "if (x < 5) print \"less\";"
        "else if (x == 5) print \"equal\";"
        "else print \"greater\";"
    );
    EXPECT_EQ(output, "equal\n");
}

TEST(Interpreter, IfWithBlock) {
    std::string output = runCode(
        "if (true) {"
        "  print \"line1\";"
        "  print \"line2\";"
        "}"
    );
    EXPECT_EQ(output, "line1\nline2\n");
}

TEST(Interpreter, NestedIf) {
    std::string output = runCode(
        "let x = 10;"
        "if (x > 5) {"
        "  if (x > 8) {"
        "    print \"very big\";"
        "  }"
        "}"
    );
    EXPECT_EQ(output, "very big\n");
}

TEST(Interpreter, IfTruthiness) {
    // Numbers: non-zero is truthy, 0 is falsy
    EXPECT_EQ(runCode("if (1) print \"yes\";"), "yes\n");
    EXPECT_EQ(runCode("if (0) print \"yes\";"), "");           // 0 is FALSY - no output
    // Strings: non-empty is truthy, empty is falsy
    EXPECT_EQ(runCode("if (\"\") print \"yes\";"), "");        // "" is FALSY - no output
    EXPECT_EQ(runCode("if (\"hello\") print \"yes\";"), "yes\n");
    // nil is always falsy
    EXPECT_EQ(runCode("if (nil) print \"yes\";"), "");
    // Booleans
    EXPECT_EQ(runCode("if (true) print \"yes\";"), "yes\n");
    EXPECT_EQ(runCode("if (false) print \"yes\";"), "");
}

// ========================================
// WHILE LOOP TESTS
// ========================================

TEST(Interpreter, WhileLoop) {
    std::string output = runCode(
        "let i = 0;"
        "while (i < 3) {"
        "  print i;"
        "  i = i + 1;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(Interpreter, WhileWithBreakCondition) {
    std::string output = runCode(
        "let x = 10;"
        "while (x > 0) {"
        "  print x;"
        "  x = x - 3;"
        "}"
    );
    EXPECT_EQ(output, "10\n7\n4\n1\n");
}

TEST(Interpreter, WhileFalseNeverExecutes) {
    std::string output = runCode(
        "while (false) {"
        "  print \"never\";"
        "}"
        "print \"done\";"
    );
    EXPECT_EQ(output, "done\n");
}

TEST(Interpreter, NestedWhile) {
    std::string output = runCode(
        "let i = 0;"
        "while (i < 2) {"
        "  let j = 0;"
        "  while (j < 2) {"
        "    print i * 10 + j;"
        "    j = j + 1;"
        "  }"
        "  i = i + 1;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n10\n11\n");
}

TEST(Interpreter, WhileWithComplexCondition) {
    std::string output = runCode(
        "let x = 0;"
        "let y = 10;"
        "while (x < 5 && y > 5) {"
        "  x = x + 1;"
        "  y = y - 1;"
        "  print x;"
        "}"
    );
    EXPECT_EQ(output, "1\n2\n3\n4\n5\n");
}

// ========================================
// FOR LOOP TESTS
// ========================================

TEST(Interpreter, BasicForLoop) {
    std::string output = runCode(
        "for (let i = 0; i < 3; i = i + 1) {"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(Interpreter, ForLoopWithExistingVariable) {
    std::string output = runCode(
        "let i = 0;"
        "for (i = 0; i < 3; i = i + 1) {"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(Interpreter, ForLoopWithoutInitializer) {
    std::string output = runCode(
        "let i = 0;"
        "for (; i < 3; i = i + 1) {"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(Interpreter, ForLoopWithoutIncrement) {
    std::string output = runCode(
        "for (let i = 0; i < 3;) {"
        "  print i;"
        "  i = i + 1;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(Interpreter, ForLoopScope) {
    std::string output = runCode(
        "let i = 100;"
        "for (let i = 0; i < 3; i = i + 1) {"
        "  print i;"
        "}"
        "print i;"
    );
    EXPECT_EQ(output, "0\n1\n2\n100\n");
}

TEST(Interpreter, NestedForLoops) {
    std::string output = runCode(
        "for (let i = 0; i < 2; i = i + 1) {"
        "  for (let j = 0; j < 2; j = j + 1) {"
        "    print i * 10 + j;"
        "  }"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n10\n11\n");
}

TEST(Interpreter, ForLoopCountdown) {
    std::string output = runCode(
        "for (let i = 3; i > 0; i = i - 1) {"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "3\n2\n1\n");
}

TEST(Interpreter, ForLoopStep) {
    std::string output = runCode(
        "for (let i = 0; i < 10; i = i + 2) {"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n2\n4\n6\n8\n");
}

// ========================================
// COMPLEX INTEGRATION TESTS
// ========================================

TEST(Interpreter, Fibonacci) {
    std::string output = runCode(
        "let a = 0;"
        "let b = 1;"
        "for (let i = 0; i < 5; i = i + 1) {"
        "  print a;"
        "  let temp = a;"
        "  a = b;"
        "  b = temp + b;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n1\n2\n3\n");
}

TEST(Interpreter, Factorial) {
    std::string output = runCode(
        "let n = 5;"
        "let result = 1;"
        "while (n > 0) {"
        "  result = result * n;"
        "  n = n - 1;"
        "}"
        "print result;"
    );
    EXPECT_EQ(output, "120\n");
}

TEST(Interpreter, SumOfNumbers) {
    std::string output = runCode(
        "let sum = 0;"
        "for (let i = 1; i <= 10; i = i + 1) {"
        "  sum = sum + i;"
        "}"
        "print sum;"
    );
    EXPECT_EQ(output, "55\n");
}

TEST(Interpreter, MaxOfThree) {
    std::string output = runCode(
        "let a = 5;"
        "let b = 12;"
        "let c = 8;"
        "let max = a;"
        "if (b > max) max = b;"
        "if (c > max) max = c;"
        "print max;"
    );
    EXPECT_EQ(output, "12\n");
}

TEST(Interpreter, EvenOddCheck) {
    std::string output = runCode(
        "for (let i = 0; i < 5; i = i + 1) {"
        "  let remainder = i % 2;"
        "  if (remainder == 0) {"
        "    print \"even\";"
        "  } else {"
        "    print \"odd\";"
        "  }"
        "}"
    );
    EXPECT_EQ(output, "even\nodd\neven\nodd\neven\n");
}

TEST(Interpreter, MultiplicationTable) {
    std::string output = runCode(
        "let n = 3;"
        "for (let i = 1; i <= 3; i = i + 1) {"
        "  print n * i;"
        "}"
    );
    EXPECT_EQ(output, "3\n6\n9\n");
}

TEST(Interpreter, NestedIfInLoop) {
    std::string output = runCode(
        "for (let i = 1; i <= 5; i = i + 1) {"
        "  if (i == 1) print \"one\";"
        "  else if (i == 2) print \"two\";"
        "  else if (i == 3) print \"three\";"
        "  else print \"many\";"
        "}"
    );
    EXPECT_EQ(output, "one\ntwo\nthree\nmany\nmany\n");
}

// Note: This test is disabled because VoltScript uses floating-point division.
// Integer truncation behavior would require explicit floor() function.
// TEST(Interpreter, CountDigits) {
//     std::string output = runCode(
//         "let n = 12345;"
//         "let count = 0;"
//         "while (n > 0) {"
//         "  n = n / 10;"
//         "  count = count + 1;"
//         "}"
//         "print count;"
//     );
//     EXPECT_EQ(output, "5\n");
// }
// Note: This test is disabled because VoltScript uses floating-point division.
// Integer truncation behavior would require explicit floor() function.
// TEST(Interpreter, ReverseNumber) {
//     std::string output = runCode(
//         "let n = 123;"
//         "let reversed = 0;"
//         "while (n > 0) {"
//         "  let digit = n % 10;"
//         "  reversed = reversed * 10 + digit;"
//         "  n = n / 10;"
//         "}"
//         "print reversed;"
//     );
//     EXPECT_EQ(output, "321\n");
// }

TEST(Interpreter, SumEvenNumbers) {
    std::string output = runCode(
        "let sum = 0;"
        "for (let i = 0; i <= 10; i = i + 1) {"
        "  if (i % 2 == 0) {"
        "    sum = sum + i;"
        "  }"
        "}"
        "print sum;"
    );
    EXPECT_EQ(output, "30\n");
}

// ========================================
// SCOPE AND SHADOWING TESTS
// ========================================

TEST(Interpreter, DeepNesting) {
    std::string output = runCode(
        "let x = 1;"
        "{"
        "  let x = 2;"
        "  {"
        "    let x = 3;"
        "    {"
        "      let x = 4;"
        "      print x;"
        "    }"
        "    print x;"
        "  }"
        "  print x;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output, "4\n3\n2\n1\n");
}

TEST(Interpreter, ShadowingInLoop) {
    std::string output = runCode(
        "let x = 100;"
        "for (let i = 0; i < 3; i = i + 1) {"
        "  let x = i;"
        "  print x;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output, "0\n1\n2\n100\n");
}

TEST(Interpreter, MultipleBlocksSequential) {
    std::string output = runCode(
        "{ let x = 1; print x; }"
        "{ let x = 2; print x; }"
        "{ let x = 3; print x; }"
    );
    EXPECT_EQ(output, "1\n2\n3\n");
}

// ========================================
// LOGICAL OPERATOR TESTS
// ========================================

TEST(Interpreter, LogicalAndShortCircuit) {
    std::string output = runCode(
        "let a = false;"
        "let b = true;"
        "if (a && b) {"
        "  print \"yes\";"
        "} else {"
        "  print \"no\";"
        "}"
    );
    EXPECT_EQ(output, "no\n");
}

TEST(Interpreter, LogicalOrShortCircuit) {
    std::string output = runCode(
        "let a = true;"
        "let b = false;"
        "if (a || b) {"
        "  print \"yes\";"
        "} else {"
        "  print \"no\";"
        "}"
    );
    EXPECT_EQ(output, "yes\n");
}

TEST(Interpreter, ComplexLogical) {
    std::string output = runCode(
        "let x = 5;"
        "if ((x > 0 && x < 10) || x == 100) {"
        "  print \"valid\";"
        "}"
    );
    EXPECT_EQ(output, "valid\n");
}

// ========================================
// STRING TESTS
// ========================================

TEST(Interpreter, StringConcatenation) {
    std::string output = runCode(
        "let first = \"Hello\";"
        "let second = \" World\";"
        "print first + second;"
    );
    EXPECT_EQ(output, "Hello World\n");
}

TEST(Interpreter, StringInLoop) {
    std::string output = runCode(
        "let str = \"\";"
        "for (let i = 0; i < 3; i = i + 1) {"
        "  str = str + \"a\";"
        "}"
        "print str;"
    );
    EXPECT_EQ(output, "aaa\n");
}

// ========================================
// ERROR TESTS
// ========================================

TEST(Interpreter, UndefinedVariableError) {
    std::string output = runCode("print x;");
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Interpreter, DivisionByZero) {
    std::string output = runCode("print 10 / 0;");
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Interpreter, TypeCoercionAddition) {
    // number + string now coerces to string concatenation
    std::string output = runCode("print 5 + \"hello\";");
    EXPECT_EQ(output, "5hello\n");
}

TEST(Interpreter, TypeMismatchComparison) {
    std::string output = runCode("print \"hello\" > 5;");
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

// ========================================
// EDGE CASES
// ========================================

TEST(Interpreter, EmptyBlock) {
    std::string output = runCode("{}");
    EXPECT_EQ(output, "");
}

TEST(Interpreter, MultipleEmptyBlocks) {
    std::string output = runCode("{}{}{} print \"done\";");
    EXPECT_EQ(output, "done\n");
}

TEST(Interpreter, LoopWithZeroIterations) {
    std::string output = runCode(
        "for (let i = 10; i < 5; i = i + 1) {"
        "  print \"never\";"
        "}"
        "print \"done\";"
    );
    EXPECT_EQ(output, "done\n");
}

TEST(Interpreter, AssignmentInCondition) {
    std::string output = runCode(
        "let x = 0;"
        "if (x = 5) {"  // Assignment, not comparison
        "  print x;"
        "}"
    );
    EXPECT_EQ(output, "5\n");
}

TEST(Interpreter, NestedForAndWhile) {
    std::string output = runCode(
        "for (let i = 0; i < 2; i = i + 1) {"
        "  let j = 0;"
        "  while (j < 2) {"
        "    print i * 10 + j;"
        "    j = j + 1;"
        "  }"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n10\n11\n");
}
