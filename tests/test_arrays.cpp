#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <iostream>
#include <sstream>

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

} // anonymous namespace

// ========================================
// ARRAY CREATION TESTS
// ========================================

TEST(Arrays, EmptyArray) {
    std::string output = runCode("let arr = []; print arr;");
    EXPECT_EQ(output, "[]\n");
}

TEST(Arrays, SingleElementArray) {
    std::string output = runCode("let arr = [42]; print arr;");
    EXPECT_EQ(output, "[42]\n");
}

TEST(Arrays, MultipleNumbers) {
    std::string output = runCode("let arr = [1, 2, 3, 4, 5]; print arr;");
    EXPECT_EQ(output, "[1, 2, 3, 4, 5]\n");
}

TEST(Arrays, StringArray) {
    std::string output = runCode("let arr = [\"hello\", \"world\"]; print arr;");
    EXPECT_EQ(output, "[hello, world]\n");
}

TEST(Arrays, MixedTypes) {
    std::string output = runCode("let arr = [1, \"two\", true, nil]; print arr;");
    EXPECT_EQ(output, "[1, two, true, nil]\n");
}

TEST(Arrays, ArrayWithExpressions) {
    std::string output = runCode("let arr = [1 + 2, 10 * 2, 5 - 3]; print arr;");
    EXPECT_EQ(output, "[3, 20, 2]\n");
}

TEST(Arrays, ArrayWithVariables) {
    std::string output = runCode(
        "let x = 10;"
        "let y = 20;"
        "let arr = [x, y, x + y];"
        "print arr;"
    );
    EXPECT_EQ(output, "[10, 20, 30]\n");
}

TEST(Arrays, TrailingComma) {
    std::string output = runCode("let arr = [1, 2, 3,]; print arr;");
    EXPECT_EQ(output, "[1, 2, 3]\n");
}

// ========================================
// ARRAY ACCESS TESTS
// ========================================

TEST(Arrays, AccessFirstElement) {
    std::string output = runCode("let arr = [10, 20, 30]; print arr[0];");
    EXPECT_EQ(output, "10\n");
}

TEST(Arrays, AccessLastElement) {
    std::string output = runCode("let arr = [10, 20, 30]; print arr[2];");
    EXPECT_EQ(output, "30\n");
}

TEST(Arrays, AccessMiddleElement) {
    std::string output = runCode("let arr = [5, 10, 15, 20]; print arr[2];");
    EXPECT_EQ(output, "15\n");
}

TEST(Arrays, AccessWithVariable) {
    std::string output = runCode(
        "let arr = [100, 200, 300];"
        "let i = 1;"
        "print arr[i];"
    );
    EXPECT_EQ(output, "200\n");
}

TEST(Arrays, AccessWithExpression) {
    std::string output = runCode(
        "let arr = [10, 20, 30, 40];"
        "print arr[1 + 1];"
    );
    EXPECT_EQ(output, "30\n");
}

TEST(Arrays, OutOfBoundsAccess) {
    std::string output = runCode("let arr = [1, 2, 3]; print arr[10];");
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Arrays, NegativeIndexAccess) {
    std::string output = runCode("let arr = [1, 2, 3]; print arr[-1];");
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

// ========================================
// ARRAY MODIFICATION TESTS
// ========================================

TEST(Arrays, ModifyFirstElement) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "arr[0] = 99;"
        "print arr;"
    );
    EXPECT_EQ(output, "[99, 2, 3]\n");
}

TEST(Arrays, ModifyLastElement) {
    std::string output = runCode(
        "let arr = [10, 20, 30];"
        "arr[2] = 777;"
        "print arr;"
    );
    EXPECT_EQ(output, "[10, 20, 777]\n");
}

TEST(Arrays, ModifyWithExpression) {
    std::string output = runCode(
        "let arr = [5, 10, 15];"
        "arr[1] = arr[1] * 2;"
        "print arr;"
    );
    EXPECT_EQ(output, "[5, 20, 15]\n");
}

TEST(Arrays, ModifyMultipleElements) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "arr[0] = 10;"
        "arr[1] = 20;"
        "arr[2] = 30;"
        "print arr;"
    );
    EXPECT_EQ(output, "[10, 20, 30]\n");
}

TEST(Arrays, ModifyOutOfBounds) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "arr[10] = 99;"
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Arrays, ModifyWithVariable) {
    std::string output = runCode(
        "let arr = [100, 200, 300];"
        "let i = 1;"
        "arr[i] = 999;"
        "print arr;"
    );
    EXPECT_EQ(output, "[100, 999, 300]\n");
}

TEST(Arrays, ChainedModification) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "arr[0] = arr[1] = arr[2] = 42;"
        "print arr;"
    );
    EXPECT_EQ(output, "[42, 42, 42]\n");
}

// ========================================
// ARRAY LENGTH TESTS
// ========================================

TEST(Arrays, LengthOfEmptyArray) {
    std::string output = runCode("let arr = []; print arr.length;");
    EXPECT_EQ(output, "0\n");
}

TEST(Arrays, LengthOfSingleElement) {
    std::string output = runCode("let arr = [42]; print arr.length;");
    EXPECT_EQ(output, "1\n");
}

TEST(Arrays, LengthOfMultipleElements) {
    std::string output = runCode("let arr = [1, 2, 3, 4, 5]; print arr.length;");
    EXPECT_EQ(output, "5\n");
}

TEST(Arrays, LengthInExpression) {
    std::string output = runCode(
        "let arr = [10, 20, 30];"
        "print arr.length * 2;"
    );
    EXPECT_EQ(output, "6\n");
}

TEST(Arrays, LengthInCondition) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "if (arr.length > 2) print \"yes\";"
    );
    EXPECT_EQ(output, "yes\n");
}

TEST(Arrays, LengthAfterModification) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "arr[1] = 99;"
        "print arr.length;"
    );
    EXPECT_EQ(output, "3\n");
}

// ========================================
// ARRAY PUSH TESTS
// ========================================

TEST(Arrays, PushSingleElement) {
    std::string output = runCode(
        "let arr = [1, 2];"
        "arr.push(3);"
        "print arr;"
    );
    EXPECT_EQ(output, "[1, 2, 3]\n");
}

TEST(Arrays, PushMultipleElements) {
    std::string output = runCode(
        "let arr = [10];"
        "arr.push(20);"
        "arr.push(30);"
        "arr.push(40);"
        "print arr;"
    );
    EXPECT_EQ(output, "[10, 20, 30, 40]\n");
}

TEST(Arrays, PushToEmptyArray) {
    std::string output = runCode(
        "let arr = [];"
        "arr.push(1);"
        "arr.push(2);"
        "print arr;"
    );
    EXPECT_EQ(output, "[1, 2]\n");
}

TEST(Arrays, PushDifferentTypes) {
    std::string output = runCode(
        "let arr = [];"
        "arr.push(42);"
        "arr.push(\"hello\");"
        "arr.push(true);"
        "print arr;"
    );
    EXPECT_EQ(output, "[42, hello, true]\n");
}

TEST(Arrays, PushUpdatesLength) {
    std::string output = runCode(
        "let arr = [1, 2];"
        "print arr.length;"
        "arr.push(3);"
        "print arr.length;"
    );
    EXPECT_EQ(output, "2\n3\n");
}

TEST(Arrays, PushInLoop) {
    std::string output = runCode(
        "let arr = [];"
        "for (let i = 1; i <= 3; i = i + 1) {"
        "  arr.push(i * 10);"
        "}"
        "print arr;"
    );
    EXPECT_EQ(output, "[10, 20, 30]\n");
}

// ========================================
// ARRAY POP TESTS
// ========================================

TEST(Arrays, PopLastElement) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "let val = arr.pop();"
        "print val;"
        "print arr;"
    );
    EXPECT_EQ(output, "3\n[1, 2]\n");
}

TEST(Arrays, PopMultipleTimes) {
    std::string output = runCode(
        "let arr = [10, 20, 30, 40];"
        "arr.pop();"
        "arr.pop();"
        "print arr;"
    );
    EXPECT_EQ(output, "[10, 20]\n");
}

TEST(Arrays, PopUntilEmpty) {
    std::string output = runCode(
        "let arr = [1, 2];"
        "arr.pop();"
        "arr.pop();"
        "print arr;"
    );
    EXPECT_EQ(output, "[]\n");
}

TEST(Arrays, PopUpdatesLength) {
    std::string output = runCode(
        "let arr = [1, 2, 3, 4];"
        "print arr.length;"
        "arr.pop();"
        "print arr.length;"
    );
    EXPECT_EQ(output, "4\n3\n");
}

TEST(Arrays, PopFromEmptyArray) {
    std::string output = runCode(
        "let arr = [];"
        "let val = arr.pop();"
        "print val;"
    );
    EXPECT_EQ(output, "nil\n");
}

TEST(Arrays, PushAndPop) {
    std::string output = runCode(
        "let arr = [];"
        "arr.push(10);"
        "arr.push(20);"
        "let val = arr.pop();"
        "print val;"
        "print arr;"
    );
    EXPECT_EQ(output, "20\n[10]\n");
}

TEST(Arrays, ReverseMethod) {
    std::string output = runCode(
        "let arr = [1, 2, 3, 4];"
        "arr.reverse();"
        "print arr;"
    );
    EXPECT_EQ(output, "[4, 3, 2, 1]\n");
}

TEST(Arrays, ReverseEmpty) {
    std::string output = runCode(
        "let arr = [];"
        "arr.reverse();"
        "print arr;"
    );
    EXPECT_EQ(output, "[]\n");
}

// ========================================
// NESTED ARRAYS TESTS
// ========================================

TEST(Arrays, SimpleNestedArray) {
    std::string output = runCode(
        "let arr = [[1, 2], [3, 4]];"
        "print arr;"
    );
    EXPECT_EQ(output, "[[1, 2], [3, 4]]\n");
}

TEST(Arrays, AccessNestedElement) {
    std::string output = runCode(
        "let matrix = [[1, 2], [3, 4]];"
        "print matrix[0][1];"
    );
    EXPECT_EQ(output, "2\n");
}

TEST(Arrays, AccessDeepNested) {
    std::string output = runCode(
        "let arr = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]];"
        "print arr[1][0][1];"
    );
    EXPECT_EQ(output, "6\n");
}

TEST(Arrays, ModifyNestedElement) {
    std::string output = runCode(
        "let matrix = [[1, 2], [3, 4]];"
        "matrix[0][1] = 99;"
        "print matrix;"
    );
    EXPECT_EQ(output, "[[1, 99], [3, 4]]\n");
}

TEST(Arrays, NestedArrayLength) {
    std::string output = runCode(
        "let matrix = [[1, 2, 3], [4, 5]];"
        "print matrix.length;"
        "print matrix[0].length;"
        "print matrix[1].length;"
    );
    EXPECT_EQ(output, "2\n3\n2\n");  // CORRECT: 2 rows, first has 3, second has 2
}

TEST(Arrays, PushNestedArray) {
    std::string output = runCode(
        "let arr = [[1, 2]];"
        "arr.push([3, 4]);"
        "print arr;"
    );
    EXPECT_EQ(output, "[[1, 2], [3, 4]]\n");
}

TEST(Arrays, PopFromNestedArray) {
    std::string output = runCode(
        "let matrix = [[1, 2, 3], [4, 5, 6]];"
        "let val = matrix[0].pop();"
        "print val;"
        "print matrix;"
    );
    EXPECT_EQ(output, "3\n[[1, 2], [4, 5, 6]]\n");
}

// ========================================
// ARRAYS IN LOOPS
// ========================================

TEST(Arrays, IterateWithForLoop) {
    std::string output = runCode(
        "let arr = [5, 10, 15];"
        "for (let i = 0; i < arr.length; i = i + 1) {"
        "  print arr[i];"
        "}"
    );
    EXPECT_EQ(output, "5\n10\n15\n");
}

TEST(Arrays, BuildArrayInLoop) {
    std::string output = runCode(
        "let squares = [];"
        "for (let i = 1; i <= 5; i = i + 1) {"
        "  squares.push(i * i);"
        "}"
        "print squares;"
    );
    EXPECT_EQ(output, "[1, 4, 9, 16, 25]\n");
}

TEST(Arrays, ModifyInLoop) {
    std::string output = runCode(
        "let arr = [1, 2, 3, 4];"
        "for (let i = 0; i < arr.length; i = i + 1) {"
        "  arr[i] = arr[i] * 2;"
        "}"
        "print arr;"
    );
    EXPECT_EQ(output, "[2, 4, 6, 8]\n");
}

TEST(Arrays, SumElements) {
    std::string output = runCode(
        "let arr = [10, 20, 30, 40];"
        "let sum = 0;"
        "for (let i = 0; i < arr.length; i = i + 1) {"
        "  sum = sum + arr[i];"
        "}"
        "print sum;"
    );
    EXPECT_EQ(output, "100\n");
}

TEST(Arrays, ReverseIteration) {
    std::string output = runCode(
        "let arr = [1, 2, 3];"
        "for (let i = arr.length - 1; i >= 0; i = i - 1) {"
        "  print arr[i];"
        "}"
    );
    EXPECT_EQ(output, "3\n2\n1\n");
}

TEST(Arrays, NestedLoopWithMatrix) {
    std::string output = runCode(
        "let matrix = [[1, 2], [3, 4]];"
        "for (let i = 0; i < matrix.length; i = i + 1) {"
        "  for (let j = 0; j < matrix[i].length; j = j + 1) {"
        "    print matrix[i][j];"
        "  }"
        "}"
    );
    EXPECT_EQ(output, "1\n2\n3\n4\n");
}

// ========================================
// ARRAYS WITH FUNCTIONS
// ========================================

TEST(Arrays, PassArrayToFunction) {
    std::string output = runCode(
        "fn printArray(arr) {"
        "  for (let i = 0; i < arr.length; i = i + 1) {"
        "    print arr[i];"
        "  }"
        "}"
        "let nums = [10, 20, 30];"
        "printArray(nums);"
    );
    EXPECT_EQ(output, "10\n20\n30\n");
}

TEST(Arrays, ReturnArrayFromFunction) {
    std::string output = runCode(
        "fn makeArray() {"
        "  let arr = [1, 2, 3];"
        "  return arr;"
        "}"
        "let result = makeArray();"
        "print result;"
    );
    EXPECT_EQ(output, "[1, 2, 3]\n");
}

TEST(Arrays, SumFunction) {
    std::string output = runCode(
        "fn sum(arr) {"
        "  let total = 0;"
        "  for (let i = 0; i < arr.length; i = i + 1) {"
        "    total = total + arr[i];"
        "  }"
        "  return total;"
        "}"
        "print sum([5, 10, 15, 20]);"
    );
    EXPECT_EQ(output, "50\n");
}

TEST(Arrays, MaxFunction) {
    std::string output = runCode(
        "fn max(arr) {"
        "  let maxVal = arr[0];"
        "  for (let i = 1; i < arr.length; i = i + 1) {"
        "    if (arr[i] > maxVal) {"
        "      maxVal = arr[i];"
        "    }"
        "  }"
        "  return maxVal;"
        "}"
        "print max([3, 7, 2, 9, 4]);"
    );
    EXPECT_EQ(output, "9\n");
}

TEST(Arrays, ReverseFunction) {
    std::string output = runCode(
        "fn reverse(arr) {"
        "  let result = [];"
        "  for (let i = arr.length - 1; i >= 0; i = i - 1) {"
        "    result.push(arr[i]);"
        "  }"
        "  return result;"
        "}"
        "print reverse([1, 2, 3, 4]);"
    );
    EXPECT_EQ(output, "[4, 3, 2, 1]\n");
}

TEST(Arrays, FilterEvens) {
    std::string output = runCode(
        "fn filterEvens(arr) {"
        "  let result = [];"
        "  for (let i = 0; i < arr.length; i = i + 1) {"
        "    if (arr[i] % 2 == 0) {"
        "      result.push(arr[i]);"
        "    }"
        "  }"
        "  return result;"
        "}"
        "print filterEvens([1, 2, 3, 4, 5, 6]);"
    );
    EXPECT_EQ(output, "[2, 4, 6]\n");
}

TEST(Arrays, MapDouble) {
    std::string output = runCode(
        "fn doubleAll(arr) {"
        "  let result = [];"
        "  for (let i = 0; i < arr.length; i = i + 1) {"
        "    result.push(arr[i] * 2);"
        "  }"
        "  return result;"
        "}"
        "print doubleAll([1, 2, 3]);"
    );
    EXPECT_EQ(output, "[2, 4, 6]\n");
}

// ========================================
// BUILT-IN LEN() WITH ARRAYS
// ========================================

TEST(Arrays, LenBuiltinFunction) {
    std::string output = runCode(
        "let arr = [1, 2, 3, 4, 5];"
        "print len(arr);"
    );
    EXPECT_EQ(output, "5\n");
}

TEST(Arrays, LenOfEmptyArray) {
    std::string output = runCode("print len([]);");
    EXPECT_EQ(output, "0\n");
}

TEST(Arrays, LenInCondition) {
    std::string output = runCode(
        "let arr = [10, 20];"
        "if (len(arr) == 2) print \"yes\";"
    );
    EXPECT_EQ(output, "yes\n");
}

// ========================================
// EDGE CASES AND ERROR HANDLING
// ========================================

TEST(Arrays, ArrayComparison) {
    std::string output = runCode(
        "let arr1 = [1, 2, 3];"
        "let arr2 = arr1;"
        "print arr1 == arr2;"
    );
    EXPECT_EQ(output, "true\n");
}

TEST(Arrays, DifferentArraysNotEqual) {
    std::string output = runCode(
        "let arr1 = [1, 2, 3];"
        "let arr2 = [1, 2, 3];"
        "print arr1 == arr2;"
    );
    EXPECT_EQ(output, "false\n");
}

TEST(Arrays, ArrayTruthiness) {
    std::string output = runCode(
        "let arr = [];"
        "if (arr) print \"truthy\";"
    );
    EXPECT_EQ(output, "");
}

TEST(Arrays, NonEmptyArrayTruthy) {
    std::string output = runCode(
        "let arr = [1];"
        "if (arr) print \"truthy\";"
    );
    EXPECT_EQ(output, "truthy\n");
}

TEST(Arrays, CannotIndexNonArray) {
    std::string output = runCode(
        "let x = 42;"
        "print x[0];"
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}

TEST(Arrays, CannotModifyNonArray) {
    std::string output = runCode(
        "let x = \"hello\";"
        "x[0] = 99;"
    );
    EXPECT_EQ(output, "RUNTIME_ERROR");
}
