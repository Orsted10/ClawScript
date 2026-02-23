#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <sstream>
#include <iostream>

// Helper to capture print output
namespace {

class PrintCapture {
public:
    PrintCapture() : old(std::cout.rdbuf(buffer.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(old); }
    std::string get() { return buffer.str(); }
private:
    std::stringstream buffer;
    std::streambuf* old;
};

// Helper to run code
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

} 

// ========================================
// BASIC FUNCTION TESTS
// ========================================

TEST(Functions, SimpleFunctionDeclaration) {
    std::string output = runCode(
        "fn greet() {"
        "  print \"Hello\";"
        "}"
        "greet();"
    );
    EXPECT_EQ(output, "Hello\n");
}

TEST(Functions, FunctionWithParameter) {
    std::string output = runCode(
        "fn greet(name) {"
        "  print \"Hello, \" + name;"
        "}"
        "greet(\"Alice\");"
    );
    EXPECT_EQ(output, "Hello, Alice\n");
}

TEST(Functions, FunctionWithMultipleParameters) {
    std::string output = runCode(
        "fn add(a, b) {"
        "  print a + b;"
        "}"
        "add(5, 3);"
    );
    EXPECT_EQ(output, "8\n");
}

TEST(Functions, FunctionWithReturn) {
    std::string output = runCode(
        "fn add(a, b) {"
        "  return a + b;"
        "}"
        "let result = add(10, 20);"
        "print result;"
    );
    EXPECT_EQ(output, "30\n");
}

TEST(Functions, FunctionReturnsNil) {
    std::string output = runCode(
        "fn doNothing() {"
        "  print \"doing nothing\";"
        "}"
        "let result = doNothing();"
        "print result;"
    );
    EXPECT_EQ(output, "doing nothing\nnil\n");
}

TEST(Functions, EmptyReturn) {
    std::string output = runCode(
        "fn early() {"
        "  print \"before\";"
        "  return;"
        "  print \"after\";"
        "}"
        "early();"
    );
    EXPECT_EQ(output, "before\n");
}

TEST(Functions, MultipleReturns) {
    std::string output = runCode(
        "fn max(a, b) {"
        "  if (a > b) return a;"
        "  return b;"
        "}"
        "print max(5, 3);"
        "print max(2, 8);"
    );
    EXPECT_EQ(output, "5\n8\n");
}

// ========================================
// SCOPE AND CLOSURE TESTS
// ========================================

TEST(Functions, FunctionAccessesGlobal) {
    std::string output = runCode(
        "let x = 10;"
        "fn printX() {"
        "  print x;"
        "}"
        "printX();"
    );
    EXPECT_EQ(output, "10\n");
}

TEST(Functions, FunctionModifiesGlobal) {
    std::string output = runCode(
        "let x = 10;"
        "fn modify() {"
        "  x = 20;"
        "}"
        "modify();"
        "print x;"
    );
    EXPECT_EQ(output, "20\n");
}

TEST(Functions, SimpleClosure) {
    std::string output = runCode(
        "fn makeGreeter(name) {"
        "  fn greet() {"
        "    print \"Hello, \" + name;"
        "  }"
        "  return greet;"
        "}"
        "let greeter = makeGreeter(\"Bob\");"
        "greeter();"
    );
    EXPECT_EQ(output, "Hello, Bob\n");
}

TEST(Functions, ClosureCapture) {
    std::string output = runCode(
        "fn outer() {"
        "  let x = 10;"
        "  fn inner() {"
        "    print x;"
        "  }"
        "  return inner;"
        "}"
        "let f = outer();"
        "f();"
    );
    EXPECT_EQ(output, "10\n");
}

TEST(Functions, ClosureModifiesCapture) {
    std::string output = runCode(
        "fn makeCounter() {"
        "  let count = 0;"
        "  fn increment() {"
        "    count = count + 1;"
        "    return count;"
        "  }"
        "  return increment;"
        "}"
        "let counter = makeCounter();"
        "print counter();"
        "print counter();"
        "print counter();"
    );
    EXPECT_EQ(output, "1\n2\n3\n");
}

TEST(Functions, MultipleClosureInstances) {
    std::string output = runCode(
        "fn makeCounter() {"
        "  let count = 0;"
        "  fn increment() {"
        "    count = count + 1;"
        "    return count;"
        "  }"
        "  return increment;"
        "}"
        "let counter1 = makeCounter();"
        "let counter2 = makeCounter();"
        "print counter1();"
        "print counter1();"
        "print counter2();"
        "print counter1();"
    );
    EXPECT_EQ(output, "1\n2\n1\n3\n");
}

// ========================================
// RECURSION TESTS
// ========================================

TEST(Functions, SimpleRecursion) {
    std::string output = runCode(
        "fn countdown(n) {"
        "  if (n <= 0) return;"
        "  print n;"
        "  countdown(n - 1);"
        "}"
        "countdown(3);"
    );
    EXPECT_EQ(output, "3\n2\n1\n");
}

TEST(Functions, FactorialRecursion) {
    std::string output = runCode(
        "fn factorial(n) {"
        "  if (n <= 1) return 1;"
        "  return n * factorial(n - 1);"
        "}"
        "print factorial(5);"
    );
    EXPECT_EQ(output, "120\n");
}

TEST(Functions, FibonacciRecursion) {
    std::string output = runCode(
        "fn fib(n) {"
        "  if (n <= 1) return n;"
        "  return fib(n - 1) + fib(n - 2);"
        "}"
        "print fib(0);"
        "print fib(1);"
        "print fib(6);"
    );
    EXPECT_EQ(output, "0\n1\n8\n");
}

// ========================================
// HIGHER-ORDER FUNCTION TESTS
// ========================================

TEST(Functions, FunctionAsArgument) {
    std::string output = runCode(
        "fn apply(f, x) {"
        "  return f(x);"
        "}"
        "fn double(n) {"
        "  return n * 2;"
        "}"
        "print apply(double, 5);"
    );
    EXPECT_EQ(output, "10\n");
}

TEST(Functions, ApplyTwice) {
    std::string output = runCode(
        "fn applyTwice(f, x) {"
        "  return f(f(x));"
        "}"
        "fn double(n) {"
        "  return n * 2;"
        "}"
        "print applyTwice(double, 3);"
    );
    EXPECT_EQ(output, "12\n");
}

TEST(Functions, ReturnFunction) {
    std::string output = runCode(
        "fn makeAdder(x) {"
        "  fn add(y) {"
        "    return x + y;"
        "  }"
        "  return add;"
        "}"
        "let add5 = makeAdder(5);"
        "print add5(10);"
        "print add5(20);"
    );
    EXPECT_EQ(output, "15\n25\n");
}

// ========================================
// NESTED FUNCTION TESTS
// ========================================

TEST(Functions, NestedFunctions) {
    std::string output = runCode(
        "fn outer() {"
        "  fn inner() {"
        "    print \"inner\";"
        "  }"
        "  inner();"
        "}"
        "outer();"
    );
    EXPECT_EQ(output, "inner\n");
}

TEST(Functions, DeepNesting) {
    std::string output = runCode(
        "fn a() {"
        "  fn b() {"
        "    fn c() {"
        "      print \"deep\";"
        "    }"
        "    c();"
        "  }"
        "  b();"
        "}"
        "a();"
    );
    EXPECT_EQ(output, "deep\n");
}

// ========================================
// ERROR TESTS
// ========================================

TEST(Functions, WrongArgumentCount) {
    std::string output = runCode(
        "fn add(a, b) {"
        "  return a + b;"
        "}"
        "add(5);"
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Functions, TooManyArguments) {
    std::string output = runCode(
        "fn greet(name) {"
        "  print name;"
        "}"
        "greet(\"Alice\", \"Bob\");"
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Functions, CallNonFunction) {
    std::string output = runCode(
        "let x = 42;"
        "x();"
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

// ========================================
// NATIVE FUNCTION TESTS
// ========================================

TEST(Functions, ClockFunction) {
    std::string output = runCode(
        "let t = clock();"
        "print t > 0;"
    );
    EXPECT_EQ(output, "true\n");
}

// ========================================
// COMPLEX INTEGRATION TESTS
// ========================================

TEST(Functions, MapFunction) {
    std::string output = runCode(
        "fn map(f, a, b, c) {"
        "  print f(a);"
        "  print f(b);"
        "  print f(c);"
        "}"
        "fn square(n) {"
        "  return n * n;"
        "}"
        "map(square, 2, 3, 4);"
    );
    EXPECT_EQ(output, "4\n9\n16\n");
}

TEST(Functions, ComposeFunctions) {
    std::string output = runCode(
        "fn compose(f, g, x) {"
        "  return f(g(x));"
        "}"
        "fn double(n) { return n * 2; }"
        "fn increment(n) { return n + 1; }"
        "print compose(double, increment, 5);"
    );
    EXPECT_EQ(output, "12\n");
}
