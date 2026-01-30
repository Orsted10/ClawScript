#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
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

// String operations tests - Part 1
TEST(StringOperations, SimpleStrings1) {
    std::string code = "print \"hello\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello\n");
}

TEST(StringOperations, SimpleStrings2) {
    std::string code = "print \"world\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "world\n");
}

TEST(StringOperations, SimpleStrings3) {
    std::string code = "print \"test\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "test\n");
}

TEST(StringOperations, SimpleStrings4) {
    std::string code = "print \"VoltScript\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "VoltScript\n");
}

TEST(StringOperations, SimpleStrings5) {
    std::string code = "print \"123\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "123\n");
}

TEST(StringOperations, SimpleStrings6) {
    std::string code = "print \"a\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "a\n");
}

TEST(StringOperations, SimpleStrings7) {
    std::string code = "print \"\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "\n");
}

TEST(StringOperations, SimpleStrings8) {
    std::string code = "print \"Hello World\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Hello World\n");
}

TEST(StringOperations, SimpleStrings9) {
    std::string code = "print \"Testing\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Testing\n");
}

TEST(StringOperations, SimpleStrings10) {
    std::string code = "print \"End\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "End\n");
}

TEST(StringOperations, StringConcatenation1) {
    std::string code = "print \"hello\" + \" \" + \"world\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello world\n");
}

TEST(StringOperations, StringConcatenation2) {
    std::string code = "print \"a\" + \"b\" + \"c\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "abc\n");
}

TEST(StringOperations, StringConcatenation3) {
    std::string code = "print \"test\" + \"123\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "test123\n");
}

TEST(StringOperations, StringConcatenation4) {
    std::string code = "print \"x\" + \"y\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "xy\n");
}

TEST(StringOperations, StringConcatenation5) {
    std::string code = "print \"start\" + \"end\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "startend\n");
}

TEST(StringOperations, StringConcatenation6) {
    std::string code = "print \"1\" + \"2\" + \"3\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "123\n");
}

TEST(StringOperations, StringConcatenation7) {
    std::string code = "print \"A\" + \"B\" + \"C\" + \"D\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "ABCD\n");
}

TEST(StringOperations, StringConcatenation8) {
    std::string code = "print \"hello\" + \"!\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello!\n");
}

TEST(StringOperations, StringConcatenation9) {
    std::string code = "print \"\" + \"test\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "test\n");
}

TEST(StringOperations, StringConcatenation10) {
    std::string code = "print \"Volt\" + \"Script\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "VoltScript\n");
}

TEST(StringOperations, StringVariables1) {
    std::string code = "let s = \"hello\"; print s;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello\n");
}

TEST(StringOperations, StringVariables2) {
    std::string code = "let msg = \"world\"; print msg;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "world\n");
}

TEST(StringOperations, StringVariables3) {
    std::string code = "let text = \"test\"; print text;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "test\n");
}

TEST(StringOperations, StringVariables4) {
    std::string code = "let greeting = \"Hello\"; print greeting + \" \" + \"User\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Hello User\n");
}

TEST(StringOperations, StringVariables5) {
    std::string code = "let prefix = \"Mr.\"; let name = \"Smith\"; print prefix + name;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Mr.Smith\n");
}

TEST(StringOperations, StringVariables6) {
    std::string code = "let a = \"A\"; let b = \"B\"; print a + b;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "AB\n");
}

TEST(StringOperations, StringVariables7) {
    std::string code = "let x = \"123\"; let y = \"456\"; print x + y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "123456\n");
}

TEST(StringOperations, StringVariables8) {
    std::string code = "let start = \"start\"; let end = \"end\"; print start + end;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "startend\n");
}

TEST(StringOperations, StringVariables9) {
    std::string code = "let empty = \"\"; let content = \"content\"; print empty + content;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "content\n");
}

TEST(StringOperations, StringVariables10) {
    std::string code = "let lang = \"Volt\"; let version = \"Script\"; print lang + version;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "VoltScript\n");
}

TEST(StringOperations, StringWithNumbers1) {
    std::string code = "print \"Number: \" + str(42);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Number: 42\n");
}

TEST(StringOperations, StringWithNumbers2) {
    std::string code = "print \"Value: \" + str(100);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Value: 100\n");
}

TEST(StringOperations, StringWithNumbers3) {
    std::string code = "print \"Count: \" + str(0);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Count: 0\n");
}

TEST(StringOperations, StringWithNumbers4) {
    std::string code = "print \"Result: \" + str(999);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Result: 999\n");
}

TEST(StringOperations, StringWithNumbers5) {
    std::string code = "print \"Score: \" + str(75);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Score: 75\n");
}

TEST(StringOperations, StringWithNumbers6) {
    std::string code = "let num = 42; print \"The number is \" + str(num);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "The number is 42\n");
}

TEST(StringOperations, StringWithNumbers7) {
    std::string code = "let x = 10; let y = 20; print \"x=\" + str(x) + \", y=\" + str(y);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "x=10, y=20\n");
}

TEST(StringOperations, StringWithNumbers8) {
    std::string code = "print \"Age: \" + str(25);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Age: 25\n");
}

TEST(StringOperations, StringWithNumbers9) {
    std::string code = "print \"Year: \" + str(2024);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Year: 2024\n");
}

TEST(StringOperations, StringWithNumbers10) {
    std::string code = "let total = 150; print \"Total: \" + str(total);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "Total: 150\n");
}

TEST(StringOperations, StringLength1) {
    std::string code = "print len(\"hello\");";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(StringOperations, StringLength2) {
    std::string code = "print len(\"test\");";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(StringOperations, StringLength3) {
    std::string code = "print len(\"\");";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(StringOperations, StringLength4) {
    std::string code = "print len(\"a\");";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(StringOperations, StringLength5) {
    std::string code = "print len(\"VoltScript\");";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(StringOperations, StringLength6) {
    std::string code = "let s = \"hello\"; print len(s);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(StringOperations, StringLength7) {
    std::string code = "let text = \"12345\"; print len(text);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(StringOperations, StringLength8) {
    std::string code = "let empty = \"\"; print len(empty);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(StringOperations, StringLength9) {
    std::string code = "print len(\"Hello World\");";
    std::string output = runCode(code);
    EXPECT_EQ(output, "11\n");
}

TEST(StringOperations, StringLength10) {
    std::string code = "let long = \"This is a long string\"; print len(long);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "21\n");
}

TEST(StringOperations, StringComparison1) {
    std::string code = "print \"hello\" == \"hello\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}

TEST(StringOperations, StringComparison2) {
    std::string code = "print \"test\" != \"other\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}

// VoltScript does not support < for strings (only numbers); expect runtime error
TEST(StringOperations, StringComparison3) {
    std::string code = "print \"a\" < \"b\";";
    std::string output = runCode(code);
    EXPECT_TRUE(output.find("RUNTIME_ERROR") != std::string::npos);
}

// VoltScript does not support > for strings (only numbers); expect runtime error
TEST(StringOperations, StringComparison4) {
    std::string code = "print \"z\" > \"a\";";
    std::string output = runCode(code);
    EXPECT_TRUE(output.find("RUNTIME_ERROR") != std::string::npos);
}

TEST(StringOperations, StringComparison5) {
    std::string code = "print \"hello\" == \"world\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "false\n");
}

TEST(StringOperations, StringComparison6) {
    std::string code = "let s1 = \"test\"; let s2 = \"test\"; print s1 == s2;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}

TEST(StringOperations, StringComparison7) {
    std::string code = "let x = \"abc\"; let y = \"def\"; print x != y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}

// VoltScript does not support < for strings (only numbers); expect runtime error
TEST(StringOperations, StringComparison8) {
    std::string code = "print \"A\" < \"B\";";
    std::string output = runCode(code);
    EXPECT_TRUE(output.find("RUNTIME_ERROR") != std::string::npos);
}

TEST(StringOperations, StringComparison9) {
    std::string code = "print \"\" == \"\";";
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}

TEST(StringOperations, StringComparison10) {
    std::string code = "let empty1 = \"\"; let empty2 = \"\"; print empty1 == empty2;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}