#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/interpreter/interpreter.h"
#include "../src/interpreter/value.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

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

// Test clock function
void testClockFunction() {
    std::cout << "Testing clock function...\n";
    
    assert(runCode("time = clock(); print(time);") == "SUCCESS");
    
    std::cout << "Clock function tests completed.\n";
}

// Test input function
void testInputFunction() {
    std::cout << "Testing input function...\n";
    
    // Since we can't easily test actual input, we'll just verify the function exists
    assert(runCode("result = input(\"Enter something: \"); print(\"Received\");") == "SUCCESS");
    
    std::cout << "Input function tests completed.\n";
}

// Test len function with different types
void testLenFunction() {
    std::cout << "Testing len function...\n";
    
    assert(runCode("result = len(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = len([1, 2, 3]); print(result);") == "SUCCESS");
    assert(runCode("result = len({ \"a\": 1, \"b\": 2 }); print(result);") == "SUCCESS");
    
    std::cout << "Len function tests completed.\n";
}

// Test string manipulation functions
void testStringManipulation() {
    std::cout << "Testing string manipulation functions...\n";
    
    // Test toUpper and toLower
    assert(runCode("result = toUpper(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = toLower(\"WORLD\"); print(result);") == "SUCCESS");
    assert(runCode("result = upper(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = lower(\"WORLD\"); print(result);") == "SUCCESS");
    
    // Test trim
    assert(runCode("result = trim(\"  hello  \"); print(result);") == "SUCCESS");
    
    // Test substr
    assert(runCode("result = substr(\"hello world\", 0, 5); print(result);") == "SUCCESS");
    assert(runCode("result = substr(\"hello world\", 6, 5); print(result);") == "SUCCESS");
    
    // Test indexOf
    assert(runCode("result = indexOf(\"hello world\", \"world\"); print(result);") == "SUCCESS");
    assert(runCode("result = indexOf(\"hello world\", \"xyz\"); print(result);") == "SUCCESS");
    
    // Test split
    assert(runCode("result = split(\"a,b,c\", \",\"); print(result.length);") == "SUCCESS");
    assert(runCode("result = split(\"hello\", \"\"); print(result.length);") == "SUCCESS");
    
    // Test replace
    assert(runCode("result = replace(\"hello world\", \"world\", \"earth\"); print(result);") == "SUCCESS");
    assert(runCode("result = replace(\"hello hello\", \"hello\", \"hi\"); print(result);") == "SUCCESS");
    
    // Test startsWith and endsWith
    assert(runCode("result = startsWith(\"hello world\", \"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = endsWith(\"hello world\", \"world\"); print(result);") == "SUCCESS");
    
    std::cout << "String manipulation tests completed.\n";
}

// Test file I/O functions
void testFileIO() {
    std::cout << "Testing file I/O functions...\n";
    
    // Create a test file
    std::ofstream testFile("test_io.txt");
    testFile << "This is a test file for VoltScript I/O operations.";
    testFile.close();
    
    // Test readFile
    assert(runCode("content = readFile(\"test_io.txt\"); print(len(content));") == "SUCCESS");
    
    // Test writeFile
    assert(runCode("result = writeFile(\"output_test.txt\", \"Hello, VoltScript!\"); print(result);") == "SUCCESS");
    
    // Test appendFile
    assert(runCode("result = appendFile(\"append_test.txt\", \"First line\"); print(result);") == "SUCCESS");
    assert(runCode("result = appendFile(\"append_test.txt\", \"Second line\"); print(result);") == "SUCCESS");
    
    // Test fileExists
    assert(runCode("result = fileExists(\"test_io.txt\"); print(result);") == "SUCCESS");
    assert(runCode("result = fileExists(\"nonexistent.txt\"); print(result);") == "SUCCESS");
    
    // Test enhanced file functions
    assert(runCode("result = exists(\"test_io.txt\"); print(result);") == "SUCCESS");
    assert(runCode("result = fileSize(\"test_io.txt\"); print(result);") == "SUCCESS");
    
    std::cout << "File I/O tests completed.\n";
}

// Test math functions
void testMathFunctions() {
    std::cout << "Testing math functions...\n";
    
    // Test basic math
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
    assert(runCode("result = sin(3.14159 / 2); print(result);") == "SUCCESS");
    
    // Test logarithmic functions
    assert(runCode("result = log(1); print(result);") == "SUCCESS");
    assert(runCode("result = log(2.71828); print(result);") == "SUCCESS");
    assert(runCode("result = exp(0); print(result);") == "SUCCESS");
    assert(runCode("result = exp(1); print(result);") == "SUCCESS");
    
    std::cout << "Math function tests completed.\n";
}

// Test date/time functions
void testDateTimeFunctions() {
    std::cout << "Testing date/time functions...\n";
    
    assert(runCode("result = now(); print(result);") == "SUCCESS");
    assert(runCode("result = formatDate(now(), \"DD/MM/YYYY\"); print(result);") == "SUCCESS");
    
    std::cout << "Date/time function tests completed.\n";
}

// Test JSON functions
void testJsonFunctions() {
    std::cout << "Testing JSON functions...\n";
    
    assert(runCode("obj = { \"name\": \"test\", \"value\": 42 }; jsonStr = jsonEncode(obj); print(len(jsonStr));") == "SUCCESS");
    assert(runCode("jsonStr = \"{\\\"name\\\":\\\"test\\\",\\\"value\\\":42}\"; obj = jsonDecode(jsonStr); print(obj.name);") == "SUCCESS");
    
    std::cout << "JSON function tests completed.\n";
}

// Test advanced string functions
void testAdvancedStringFunctions() {
    std::cout << "Testing advanced string functions...\n";
    
    // Test padStart
    assert(runCode("result = padStart(\"hello\", 10, \"0\"); print(len(result));") == "SUCCESS");
    assert(runCode("result = padStart(\"hello\", 10, \"xy\"); print(len(result));") == "SUCCESS");
    assert(runCode("result = padStart(\"hello\", 3, \"0\"); print(len(result));") == "SUCCESS");
    
    // Test padEnd
    assert(runCode("result = padEnd(\"hello\", 10, \"0\"); print(len(result));") == "SUCCESS");
    assert(runCode("result = padEnd(\"hello\", 10, \"xy\"); print(len(result));") == "SUCCESS");
    assert(runCode("result = padEnd(\"hello\", 3, \"0\"); print(len(result));") == "SUCCESS");
    
    // Test repeat
    assert(runCode("result = repeat(\"abc\", 3); print(result);") == "SUCCESS");
    assert(runCode("result = repeat(\"x\", 5); print(result);") == "SUCCESS");
    assert(runCode("result = repeat(\"\", 5); print(result);") == "SUCCESS");
    assert(runCode("result = repeat(\"test\", 0); print(result);") == "SUCCESS");
    
    // Test charCodeAt
    assert(runCode("result = charCodeAt(\"ABC\", 0); print(result);") == "SUCCESS");
    assert(runCode("result = charCodeAt(\"ABC\", 1); print(result);") == "SUCCESS");
    assert(runCode("result = charCodeAt(\"ABC\", 10); print(result);") == "SUCCESS");
    
    // Test fromCharCode
    assert(runCode("result = fromCharCode(65); print(result);") == "SUCCESS");
    assert(runCode("result = fromCharCode(97); print(result);") == "SUCCESS");
    assert(runCode("result = fromCharCode(48); print(result);") == "SUCCESS");
    
    std::cout << "Advanced string function tests completed.\n";
}

// Test functional programming utilities
void testFunctionalUtilities() {
    std::cout << "Testing functional programming utilities...\n";
    
    // Test compose
    assert(runCode(R"(
        addOne = fun(x) { return x + 1; };
        multiplyByTwo = fun(x) { return x * 2; };
        composed = compose(addOne, multiplyByTwo);
        result = composed(5);
        print(result);
    )") == "SUCCESS");
    
    // Test pipe
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
void testPerformanceUtilities() {
    std::cout << "Testing performance utilities...\n";
    
    // Test sleep
    assert(runCode("result = sleep(50); print(result);") == "SUCCESS");
    
    // Test benchmark
    assert(runCode(R"(
        testFunc = fun(x) { return x * x; };
        benchResult = benchmark(testFunc, 5);
        print(benchResult.result);
    )") == "SUCCESS");
    
    assert(runCode(R"(
        slowFunc = fun(n) {
            sum = 0;
            for (let i = 0; i < n; i = i + 1) {
                sum = sum + i;
            }
            return sum;
        };
        benchResult = benchmark(slowFunc, 100);
        print(benchResult.timeMicroseconds > 0);
    )") == "SUCCESS");
    
    std::cout << "Performance utility tests completed.\n";
}

} // anonymous namespace
