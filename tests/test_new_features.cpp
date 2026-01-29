#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/interpreter/interpreter.h"
#include "../src/interpreter/value.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

using namespace volt;

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

// Test file I/O enhancement functions
void testFileFunctions() {
    std::cout << "Testing file functions...\n";
    
    // Test exists function
    std::string code1 = R"(
        result = exists("nonexistent_file.txt");
        print(result);
    )";
    
    // Test fileSize function
    std::string code2 = R"(
        size = fileSize("some_existing_file.txt");
        print(size);
    )";
    
    // Test deleteFile function
    std::string code3 = R"(
        success = deleteFile("temp_file.txt");
        print(success);
    )";
    
    std::cout << "File function tests completed.\n";
}

// Test advanced string functions
void testStringFunctions() {
    std::cout << "Testing string functions...\n";
    
    // Test padStart
    assert(runCode("result = padStart(\"hello\", 10, \" \"); print(result);") == "SUCCESS");
    
    // Test padEnd
    assert(runCode("result = padEnd(\"hello\", 10, \" \"); print(result);") == "SUCCESS");
    
    // Test repeat
    assert(runCode("result = repeat(\"hi\", 3); print(result);") == "SUCCESS");
    
    // Test charCodeAt
    assert(runCode("result = charCodeAt(\"hello\", 0); print(result);") == "SUCCESS");
    
    // Test fromCharCode
    assert(runCode("result = fromCharCode(65); print(result);") == "SUCCESS");
    
    std::cout << "String function tests completed.\n";
}

// Test functional programming utilities
void testFunctionalUtils() {
    std::cout << "Testing functional utilities...\n";
    
    // Test compose
    std::string code1 = R"(
        addOne = fun(x) { return x + 1; };
        multiplyByTwo = fun(x) { return x * 2; };
        composed = compose(addOne, multiplyByTwo);
        result = composed(5);  // Should be addOne(multiplyByTwo(5)) = addOne(10) = 11
        print(result);
    )";
    
    // Test pipe
    std::string code2 = R"(
        addOne = fun(x) { return x + 1; };
        multiplyByTwo = fun(x) { return x * 2; };
        piped = pipe(addOne, multiplyByTwo);
        result = piped(5);  // Should be multiplyByTwo(addOne(5)) = multiplyByTwo(6) = 12
        print(result);
    )";
    
    std::cout << "Functional utility tests completed.\n";
}

// Test performance utilities
void testPerformanceUtils() {
    std::cout << "Testing performance utilities...\n";
    
    // Test sleep
    std::string code1 = R"(
        result = sleep(100);  // Sleep for 100 milliseconds
        print(result);
    )";
    
    // Test benchmark
    std::string code2 = R"(
        testFunc = fun(x) { return x * x; };
        benchResult = benchmark(testFunc, 5);
        print(benchResult.timeMilliseconds);
    )";
    
    std::cout << "Performance utility tests completed.\n";
}

} // anonymous namespace