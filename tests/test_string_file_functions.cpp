#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/interpreter/interpreter.h"
#include "../src/interpreter/value.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <gtest/gtest.h>

using namespace claw;

namespace {

// Helper function to run VoltScript code and return the result
std::string runCode(const std::string& code) {
    try {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        if (parser.hadError()) {
            return "PARSE_ERROR";
        }
        
        Interpreter interpreter;
        interpreter.execute(statements);
        
        // For now, just return success indicator
        return "SUCCESS";
    } catch (const std::exception& e) {
        return std::string("RUNTIME_ERROR: ") + e.what();
    }
}

// Test all string functions
void testStringFunctions() {
    std::cout << "Testing string functions...\n";
    
    // Test basic string functions
    assert(runCode("result = toUpper(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = toLower(\"HELLO\"); print(result);") == "SUCCESS");
    assert(runCode("result = upper(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = lower(\"HELLO\"); print(result);") == "SUCCESS");
    assert(runCode("result = trim(\"  hello  \"); print(result);") == "SUCCESS");
    assert(runCode("result = substr(\"hello world\", 0, 5); print(result);") == "SUCCESS");
    assert(runCode("result = indexOf(\"hello world\", \"world\"); print(result);") == "SUCCESS");
    assert(runCode("result = split(\"a,b,c\", \",\"); print(result[0]);") == "SUCCESS");
    assert(runCode("result = replace(\"hello world\", \"world\", \"earth\"); print(result);") == "SUCCESS");
    assert(runCode("result = startsWith(\"hello world\", \"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = endsWith(\"hello world\", \"world\"); print(result);") == "SUCCESS");
    
    // Test advanced string functions
    assert(runCode("result = padStart(\"hello\", 10, \"0\"); print(result);") == "SUCCESS");
    assert(runCode("result = padEnd(\"hello\", 10, \"0\"); print(result);") == "SUCCESS");
    assert(runCode("result = repeat(\"abc\", 3); print(result);") == "SUCCESS");
    assert(runCode("result = charCodeAt(\"hello\", 0); print(result);") == "SUCCESS");
    assert(runCode("result = fromCharCode(65); print(result);") == "SUCCESS");
    
    std::cout << "String function tests completed.\n";
}

// Test all file functions
void testFileFunctions() {
    std::cout << "Testing file functions...\n";
    
    // Create a temporary file for testing
    std::ofstream tempFile("temp_test.txt");
    tempFile << "This is a test file.";
    tempFile.close();
    
    // Test basic file functions
    assert(runCode("result = readFile(\"temp_test.txt\"); print(result);") == "SUCCESS");
    assert(runCode("result = fileExists(\"temp_test.txt\"); print(result);") == "SUCCESS");
    assert(runCode("result = writeFile(\"temp_output.txt\", \"Hello World\"); print(result);") == "SUCCESS");
    assert(runCode("result = appendFile(\"temp_append.txt\", \"Hello\"); print(result);") == "SUCCESS");
    
    // Test enhanced file functions
    assert(runCode("result = exists(\"temp_test.txt\"); print(result);") == "SUCCESS");
    assert(runCode("result = fileSize(\"temp_test.txt\"); print(result);") == "SUCCESS");
    assert(runCode("result = deleteFile(\"temp_test.txt\"); print(result);") == "SUCCESS");
    
    std::cout << "File function tests completed.\n";
}

// Test type checking functions
void testTypeFunctions() {
    std::cout << "Testing type functions...\n";
    
    assert(runCode("result = type(42); print(result);") == "SUCCESS");
    assert(runCode("result = type(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = type(true); print(result);") == "SUCCESS");
    assert(runCode("result = type(nil); print(result);") == "SUCCESS");
    assert(runCode("result = type([1, 2, 3]); print(result);") == "SUCCESS");
    assert(runCode("result = type({ \"a\": 1 }); print(result);") == "SUCCESS");
    
    std::cout << "Type function tests completed.\n";
}

// Test math functions
void testMathFunctions() {
    std::cout << "Testing math functions...\n";
    
    // Test basic math functions
    assert(runCode("result = abs(-5); print(result);") == "SUCCESS");
    assert(runCode("result = sqrt(16); print(result);") == "SUCCESS");
    assert(runCode("result = pow(2, 3); print(result);") == "SUCCESS");
    assert(runCode("result = min(5, 3); print(result);") == "SUCCESS");
    assert(runCode("result = max(5, 3); print(result);") == "SUCCESS");
    assert(runCode("result = round(3.7); print(result);") == "SUCCESS");
    assert(runCode("result = floor(3.7); print(result);") == "SUCCESS");
    assert(runCode("result = ceil(3.7); print(result);") == "SUCCESS");
    assert(runCode("result = random(); print(result);") == "SUCCESS");
    
    // Test trigonometric functions
    assert(runCode("result = sin(0); print(result);") == "SUCCESS");
    assert(runCode("result = cos(0); print(result);") == "SUCCESS");
    assert(runCode("result = tan(0); print(result);") == "SUCCESS");
    
    // Test logarithmic functions
    assert(runCode("result = log(1); print(result);") == "SUCCESS");
    assert(runCode("result = exp(0); print(result);") == "SUCCESS");
    
    std::cout << "Math function tests completed.\n";
}

// Test date/time functions
void testDateTimeFunctions() {
    std::cout << "Testing date/time functions...\n";
    
    assert(runCode("result = now(); print(result);") == "SUCCESS");
    assert(runCode("result = formatDate(now(), \"YYYY-MM-DD\"); print(result);") == "SUCCESS");
    
    std::cout << "Date/time function tests completed.\n";
}

// Test functional programming utilities
void testFunctionalUtils() {
    std::cout << "Testing functional programming utilities...\n";
    
    assert(runCode(R"(
        addOne = fun(x) { return x + 1; };
        multiplyByTwo = fun(x) { return x * 2; };
        composed = compose(addOne, multiplyByTwo);
        result = composed(5);
        print(result);
    )") == "SUCCESS");
    
    assert(runCode(R"(
        addOne = fun(x) { return x + 1; };
        multiplyByTwo = fun(x) { return x * 2; };
        piped = pipe(addOne, multiplyByTwo);
        result = piped(5);
        print(result);
    )") == "SUCCESS");
    
    std::cout << "Functional utility tests completed.\n";
}

// Test performance utilities
void testPerformanceUtils() {
    std::cout << "Testing performance utilities...\n";
    
    assert(runCode("result = sleep(10); print(result);") == "SUCCESS");
    
    assert(runCode(R"(
        testFunc = fun(x) { return x * x; };
        benchResult = benchmark(testFunc, 5);
        print(benchResult.result);
    )") == "SUCCESS");
    
    std::cout << "Performance utility tests completed.\n";
}

} // anonymous namespace

TEST(EncryptedIO, RoundTrip) {
    ASSERT_EQ(runCode(R"(
        writeFileEnc("enc_test.bin", "hello secret", "correct horse battery staple");
        msg = readFileEnc("enc_test.bin", "correct horse battery staple");
        print(msg);
    )"), "SUCCESS");
}

TEST(EncryptedIO, SanityNew) {
    ASSERT_EQ(runCode(R"(
        print("ok");
    )"), "SUCCESS");
}

TEST(TLSPrimitives2, GatingDisabled) {
    ASSERT_EQ(runCode(R"(
        print(tlsGet("https://example.com/"));
    )"), "RUNTIME_ERROR: Network disabled by sandbox");
}

TEST(TLSPrimitives, HeadersOptional) {
    std::string res = runCode(R"(
        writeFile(".voltsec", "network=allow");
        policyReload();
        print(tlsGet("https://example.com/", {"User-Agent":"ClawTest"}));
    )");
    ASSERT_NE(res, "RUNTIME_ERROR: Network disabled by sandbox");
}

TEST(LogWrite2, MetadataRequiredPolicy) {
    ASSERT_EQ(runCode(R"(
        writeFile(".voltsec", "log.meta.required=true");
        policyReload();
        logWrite("no-meta");
        print("done");
    )"), "RUNTIME_ERROR: Log metadata required by policy");
}

TEST(LogWrite2, HmacAndMetadataWritten) {
    std::string res = runCode(R"(
        writeFile(".voltsec", "log.path=test_log.txt\nlog.hmac=abc123\noutput=allow");
        policyReload();
        logWrite("hello", {"a":1});
    )");
    ASSERT_EQ(res, "SUCCESS");
    std::ifstream f("test_log.txt");
    ASSERT_TRUE(f.good());
    std::stringstream buf; buf << f.rdbuf();
    std::string out = buf.str();
    ASSERT_NE(out.find("hello|"), std::string::npos);
    ASSERT_NE(out.find("{"), std::string::npos);
    ASSERT_NE(out.find("}"), std::string::npos);
}
