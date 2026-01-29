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

// Test array methods
void testArrayMethods() {
    std::cout << "Testing array methods...\n";
    
    // Test map
    assert(runCode(R"(
        arr = [1, 2, 3];
        doubled = arr.map(fun(x) { return x * 2; });
        print(doubled[0]);
    )") == "SUCCESS");
    
    // Test filter
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5, 6];
        evens = arr.filter(fun(x) { return x % 2 == 0; });
        print(evens[0]);
    )") == "SUCCESS");
    
    // Test reduce
    assert(runCode(R"(
        arr = [1, 2, 3, 4];
        sum = arr.reduce(fun(acc, x) { return acc + x; }, 0);
        print(sum);
    )") == "SUCCESS");
    
    // Test find
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        found = arr.find(fun(x) { return x > 3; });
        print(found);
    )") == "SUCCESS");
    
    // Test sort
    assert(runCode(R"(
        arr = [3, 1, 4, 1, 5];
        sorted = arr.sort();
        print(sorted[0]);
    )") == "SUCCESS");
    
    // Test join
    assert(runCode(R"(
        arr = ["a", "b", "c"];
        joined = arr.join(",");
        print(joined);
    )") == "SUCCESS");
    
    std::cout << "Array method tests completed.\n";
}

// Test hash map methods
void testHashMapMethods() {
    std::cout << "Testing hash map methods...\n";
    
    // Test keys
    assert(runCode(R"(
        hm = { "a": 1, "b": 2 };
        keys = hm.keys();
        print(keys[0]);
    )") == "SUCCESS");
    
    // Test values
    assert(runCode(R"(
        hm = { "a": 1, "b": 2 };
        values = hm.values();
        print(values[0]);
    )") == "SUCCESS");
    
    // Test has
    assert(runCode(R"(
        hm = { "a": 1 };
        exists = hm.has("a");
        print(exists);
    )") == "SUCCESS");
    
    // Test remove
    assert(runCode(R"(
        hm = { "a": 1, "b": 2 };
        result = hm.remove("a");
        print(result);
    )") == "SUCCESS");
    
    std::cout << "Hash map method tests completed.\n";
}

// Test JSON functions
void testJsonFunctions() {
    std::cout << "Testing JSON functions...\n";
    
    // Test jsonEncode
    assert(runCode(R"(
        obj = { "name": "test", "value": 42 };
        jsonStr = jsonEncode(obj);
        print(jsonStr);
    )") == "SUCCESS");
    
    // Test jsonDecode
    assert(runCode(R"(
        jsonStr = "{\"name\":\"test\",\"value\":42}";
        obj = jsonDecode(jsonStr);
        print(obj.name);
    )") == "SUCCESS");
    
    std::cout << "JSON function tests completed.\n";
}

// Test math functions
void testMathFunctions() {
    std::cout << "Testing math functions...\n";
    
    // Test trigonometric functions
    assert(runCode("result = sin(0); print(result);") == "SUCCESS");
    assert(runCode("result = cos(0); print(result);") == "SUCCESS");
    assert(runCode("result = tan(0); print(result);") == "SUCCESS");
    
    // Test logarithmic functions
    assert(runCode("result = log(1); print(result);") == "SUCCESS");
    assert(runCode("result = exp(0); print(result);") == "SUCCESS");
    
    // Test rounding functions
    assert(runCode("result = round(3.7); print(result);") == "SUCCESS");
    assert(runCode("result = floor(3.7); print(result);") == "SUCCESS");
    assert(runCode("result = ceil(3.7); print(result);") == "SUCCESS");
    
    std::cout << "Math function tests completed.\n";
}

// Test date/time functions
void testDateTimeFunctions() {
    std::cout << "Testing date/time functions...\n";
    
    // Test now
    assert(runCode("timestamp = now(); print(timestamp);") == "SUCCESS");
    
    // Test formatDate
    assert(runCode("formatted = formatDate(now(), \"YYYY-MM-DD\"); print(formatted);") == "SUCCESS");
    
    std::cout << "Date/time function tests completed.\n";
}

} // anonymous namespace