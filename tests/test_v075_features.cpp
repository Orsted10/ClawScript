#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <sstream>
#include <thread>
#include <chrono>

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

// ==================== v0.7.5 FEATURE TESTS ====================

// Advanced Array Methods Tests
TEST(V075Features, ArrayMap) {
    std::string code = R"(
        let numbers = [1, 2, 3, 4];
        let doubled = numbers.map(fn(x) { return x * 2; });
        print doubled[0];
        print doubled[1];
        print doubled[2];
        print doubled[3];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n6\n8\n");
}

TEST(V075Features, ArrayFilter) {
    std::string code = R"(
        let numbers = [1, 2, 3, 4, 5];
        let evens = numbers.filter(fn(x) { return x % 2 == 0; });
        print evens[0];
        print evens[1];
        print evens.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n2\n");
}

TEST(V075Features, ArrayReduce) {
    std::string code = R"(
        let numbers = [1, 2, 3, 4];
        let sum = numbers.reduce(fn(acc, curr) { return acc + curr; }, 0);
        let product = numbers.reduce(fn(acc, curr) { return acc * curr; }, 1);
        print sum;
        print product;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n24\n");
}

TEST(V075Features, ArrayFind) {
    std::string code = R"(
        let numbers = [10, 20, 30, 40];
        let found = numbers.find(fn(x) { return x > 25; });
        let notFound = numbers.find(fn(x) { return x > 100; });
        print found;
        print notFound;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "30\nnil\n");
}

TEST(V075Features, ArraySomeEvery) {
    std::string code = R"(
        let numbers = [2, 4, 6, 8];
        let hasEven = numbers.some(fn(x) { return x % 2 == 0; });
        let allEven = numbers.every(fn(x) { return x % 2 == 0; });
        let hasOdd = numbers.some(fn(x) { return x % 2 == 1; });
        print hasEven;
        print allEven;
        print hasOdd;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\ntrue\nfalse\n");
}

TEST(V075Features, ArraySliceConcatJoin) {
    std::string code = R"(
        let numbers = [1, 2, 3, 4, 5];
        let sliced = numbers.slice(1, 4);
        let other = [6, 7];
        let combined = numbers.concat(other);
        let joined = numbers.join("-");
        
        print sliced[0];
        print sliced[1];
        print sliced[2];
        print combined.length;
        print joined;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n3\n4\n7\n1-2-3-4-5\n");
}

// Date/Time Function Tests
TEST(V075Features, NowFunction) {
    std::string code = R"(
        let timestamp = now();
        print (timestamp > 1000000000000);  // Should be a large number (milliseconds since epoch)
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}

TEST(V075Features, DateFormatting) {
    std::string code = R"(
        let timestamp = parseDate("2024-01-15");
        let formatted = formatDate(timestamp, "%Y-%m-%d");
        print formatted;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2024-01-15\n");
}

TEST(V075Features, DateParsing) {
    std::string code = R"(
        let timestamp = parseDate("2024-01-15");
        let formatted = formatDate(timestamp, "%Y");
        print formatted;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2024\n");
}

TEST(V075Features, SleepFunction) {
    std::string code = R"(
        let start = now();
        sleep(10);  // Sleep for 10 milliseconds
        let end = now();
        let duration = end - start;
        print (duration >= 10);  // Should have slept for at least 10ms
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n");
}
