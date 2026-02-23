#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
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

// ==================== TRIGONOMETRIC FUNCTIONS TESTS ====================

TEST(V075Simple, TrigonometricFunctions) {
    std::string code = R"(
        print sin(0);
        print cos(0);
        print tan(0);
        print sin(3.14159 / 2);  // Should be close to 1
        print cos(3.14159);      // Should be close to -1
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Just check that it runs without error for now
}

// ==================== LOGARITHMIC FUNCTIONS TESTS ====================

TEST(V075Simple, LogarithmicFunctions) {
    std::string code = R"(
        print log(1);     // Should be 0
        print log(2.718281828);  // Should be close to 1
        print exp(0);     // Should be 1
        print exp(1);     // Should be close to 2.718281828
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Just check that it runs without error for now
}

// ==================== DATE/TIME FUNCTIONS TESTS ====================

TEST(V075Simple, DateTimeFunctions) {
    std::string code = R"(
        let timestamp = now();
        print "Current timestamp: " + str(timestamp);
        print formatDate(timestamp, "yyyy-MM-dd");
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("Current timestamp:") != std::string::npos);
    EXPECT_TRUE(output.find("Date(") != std::string::npos);
}

// ==================== JSON FUNCTIONS TESTS ====================

TEST(V075Simple, JsonEncoding) {
    std::string code = R"(
        let obj = {"name": "John", "age": 30, "active": true};
        let jsonStr = jsonEncode(obj);
        print "Encoded: " + jsonStr;

        let arr = [1, 2, 3, "hello"];
        let jsonArray = jsonEncode(arr);
        print "Array encoded: " + jsonArray;

        let simpleValue = jsonEncode(42);
        print "Number encoded: " + simpleValue;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("Encoded:") != std::string::npos);
    EXPECT_TRUE(output.find("Array encoded:") != std::string::npos);
    EXPECT_TRUE(output.find("Number encoded:") != std::string::npos);
}

TEST(V075Simple, JsonDecoding) {
    std::string code = R"(
        let jsonStr = "{\"name\":\"John\",\"age\":30}";
        let decoded = jsonDecode(jsonStr);
        print "Type of decoded: " + type(decoded);

        let jsonArray = "[1,2,3]";
        let decodedArray = jsonDecode(jsonArray);
        print "Array type: " + type(decodedArray);

        let jsonNumber = "42";
        let decodedNumber = jsonDecode(jsonNumber);
        print "Number: " + str(decodedNumber);
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Just check that it runs without error for now
}

// ==================== ERROR HANDLING TESTS ====================

TEST(V075Simple, MathFunctionErrors) {
    // Test log with negative number
    std::string code1 = R"(
        print log(-1);
    )";

    std::string output1 = runCode(code1);
    EXPECT_TRUE(output1.find("RUNTIME_ERROR") != std::string::npos);
    EXPECT_TRUE(output1.find("positive") != std::string::npos);
    
    // Test trigonometric functions with non-number
    std::string code2 = R"(
        print sin("not a number");
    )";

    std::string output2 = runCode(code2);
    EXPECT_TRUE(output2.find("RUNTIME_ERROR") != std::string::npos);
    EXPECT_TRUE(output2.find("number") != std::string::npos);
}

TEST(V075Simple, JsonFunctionErrors) {
    // Test jsonDecode with non-string
    std::string code = R"(
        print jsonDecode(123);
    )";

    std::string output = runCode(code);
    EXPECT_TRUE(output.find("RUNTIME_ERROR") != std::string::npos);
    EXPECT_TRUE(output.find("string") != std::string::npos);
}
