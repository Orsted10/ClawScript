#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/interpreter/interpreter.h"
#include "../src/interpreter/value.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

// Comprehensive array tests
void testArrayOperations() {
    std::cout << "Testing array operations...\n";
    
    // Test array creation and basic operations
    assert(runCode("arr = []; print(arr.length);") == "SUCCESS");
    assert(runCode("arr = [1, 2, 3]; print(arr.length);") == "SUCCESS");
    assert(runCode("arr = [1, 2, 3]; print(arr[0]);") == "SUCCESS");
    assert(runCode("arr = [1, 2, 3]; arr[0] = 10; print(arr[0]);") == "SUCCESS");
    
    // Test push and pop
    assert(runCode("arr = []; arr.push(1); print(arr.length);") == "SUCCESS");
    assert(runCode("arr = [1, 2]; item = arr.pop(); print(item);") == "SUCCESS");
    assert(runCode("arr = []; arr.push(1); arr.push(2); print(arr.length);") == "SUCCESS");
    
    // Test reverse
    assert(runCode("arr = [1, 2, 3]; arr.reverse(); print(arr[0]);") == "SUCCESS");
    
    // Test complex array operations
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        sum = 0;
        for (let i = 0; i < arr.length; i = i + 1) {
            sum = sum + arr[i];
        }
        print(sum);
    )") == "SUCCESS");
    
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        result = [];
        for (let i = 0; i < arr.length; i = i + 1) {
            result.push(arr[i] * 2);
        }
        print(result[0]);
    )") == "SUCCESS");
    
    // Nested arrays
    assert(runCode("arr = [[1, 2], [3, 4]]; print(arr[0][0]);") == "SUCCESS");
    assert(runCode("arr = [[1, 2], [3, 4]]; arr[0][0] = 99; print(arr[0][0]);") == "SUCCESS");
    
    // Array with mixed types
    assert(runCode("arr = [1, \"hello\", true, nil]; print(arr[1]);") == "SUCCESS");
    
    std::cout << "Array operations tests completed.\n";
}

// Test hash map operations
void testHashMapOperations() {
    std::cout << "Testing hash map operations...\n";
    
    // Test hash map creation and basic operations
    assert(runCode("hm = {}; print(hm.size);") == "SUCCESS");
    assert(runCode("hm = { \"a\": 1, \"b\": 2 }; print(hm.size);") == "SUCCESS");
    assert(runCode("hm = { \"a\": 1 }; print(hm.a);") == "SUCCESS");
    assert(runCode("hm = { \"a\": 1 }; hm.a = 10; print(hm.a);") == "SUCCESS");
    
    // Test accessing non-existent keys
    assert(runCode("hm = { \"a\": 1 }; print(hm.b == nil);") == "SUCCESS");
    
    // Test keys and values
    assert(runCode("hm = { \"a\": 1, \"b\": 2 }; keys = hm.keys(); print(keys[0]);") == "SUCCESS");
    assert(runCode("hm = { \"a\": 1, \"b\": 2 }; vals = hm.values(); print(vals[0]);") == "SUCCESS");
    
    // Test has and remove
    assert(runCode("hm = { \"a\": 1 }; exists = hm.has(\"a\"); print(exists);") == "SUCCESS");
    assert(runCode("hm = { \"a\": 1 }; exists = hm.has(\"b\"); print(exists);") == "SUCCESS");
    assert(runCode("hm = { \"a\": 1, \"b\": 2 }; removed = hm.remove(\"a\"); print(removed);") == "SUCCESS");
    
    // Complex hash map operations
    assert(runCode(R"(
        hm = { "name": "John", "age": 30 };
        greeting = "Hello " + hm.name + ", you are " + str(hm.age) + " years old";
        print(greeting);
    )") == "SUCCESS");
    
    // Nested hash maps
    assert(runCode(R"(
        nested = {
            "person": {
                "name": "Alice",
                "age": 25
            },
            "location": {
                "city": "Paris",
                "country": "France"
            }
        };
        print(nested.person.name);
    )") == "SUCCESS");
    
    std::cout << "Hash map operations tests completed.\n";
}

// Test loops and control flow with arrays
void testControlFlowWithArrays() {
    std::cout << "Testing control flow with arrays...\n";
    
    // For loop with arrays
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        sum = 0;
        for (let i in arr) {
            sum = sum + i;
        }
        print(sum);
    )") == "SUCCESS");
    
    // While loop with arrays
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        i = 0;
        sum = 0;
        while (i < arr.length) {
            sum = sum + arr[i];
            i = i + 1;
        }
        print(sum);
    )") == "SUCCESS");
    
    // Conditional logic with arrays
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        result = [];
        for (let i = 0; i < arr.length; i = i + 1) {
            if (arr[i] % 2 == 0) {
                result.push(arr[i]);
            }
        }
        print(result.length);
    )") == "SUCCESS");
    
    // Run-until loop with arrays
    assert(runCode(R"(
        arr = [1, 2, 3, 4, 5];
        i = 0;
        while (true) {
            if (i >= arr.length) break;
            i = i + 1;
        }
        print(i);
    )") == "SUCCESS");
    
    std::cout << "Control flow with arrays tests completed.\n";
}

// Test functions with arrays and hash maps
void testFunctionsWithCollections() {
    std::cout << "Testing functions with collections...\n";
    
    // Function that takes array
    assert(runCode(R"(
        sumArray = fun(arr) {
            sum = 0;
            for (let i = 0; i < arr.length; i = i + 1) {
                sum = sum + arr[i];
            }
            return sum;
        };
        result = sumArray([1, 2, 3, 4, 5]);
        print(result);
    )") == "SUCCESS");
    
    // Function that takes hash map
    assert(runCode(R"(
        getName = fun(person) {
            return person.name;
        };
        person = { "name": "Bob", "age": 40 };
        result = getName(person);
        print(result);
    )") == "SUCCESS");
    
    // Function that returns array
    assert(runCode(R"(
        createSequence = fun(n) {
            result = [];
            for (let i = 1; i <= n; i = i + 1) {
                result.push(i);
            }
            return result;
        };
        seq = createSequence(5);
        print(seq[0]);
    )") == "SUCCESS");
    
    // Function that returns hash map
    assert(runCode(R"(
        createUser = fun(name, age) {
            return {
                "name": name,
                "age": age,
                "active": true
            };
        };
        user = createUser("Charlie", 35);
        print(user.name);
    )") == "SUCCESS");
    
    // Higher-order function with arrays
    assert(runCode(R"(
        applyFunc = fun(arr, func) {
            result = [];
            for (let i = 0; i < arr.length; i = i + 1) {
                result.push(func(arr[i]));
            }
            return result;
        };
        doubled = applyFunc([1, 2, 3], fun(x) { return x * 2; });
        print(doubled[0]);
    )") == "SUCCESS");
    
    std::cout << "Functions with collections tests completed.\n";
}

// Test edge cases and error handling
void testEdgeCases() {
    std::cout << "Testing edge cases...\n";
    
    // Empty arrays
    assert(runCode("arr = []; print(arr.length);") == "SUCCESS");
    
    // Single element arrays
    assert(runCode("arr = [42]; print(arr[0]);") == "SUCCESS");
    
    // Empty hash maps
    assert(runCode("hm = {}; print(hm.size);") == "SUCCESS");
    
    // Single key hash maps
    assert(runCode("hm = { \"key\": \"value\" }; print(hm.key);") == "SUCCESS");
    
    // Very large arrays (within reason)
    assert(runCode(R"(
        arr = [];
        for (let i = 0; i < 10; i = i + 1) {
            arr.push(i);
        }
        print(arr.length);
    )") == "SUCCESS");
    
    // Arrays with different types
    assert(runCode("mixed = [1, \"two\", true, 4.0, nil]; print(type(mixed[0]));") == "SUCCESS");
    assert(runCode("mixed = [1, \"two\", true, 4.0, nil]; print(type(mixed[1]));") == "SUCCESS");
    assert(runCode("mixed = [1, \"two\", true, 4.0, nil]; print(type(mixed[2]));") == "SUCCESS");
    assert(runCode("mixed = [1, \"two\", true, 4.0, nil]; print(type(mixed[3]));") == "SUCCESS");
    assert(runCode("mixed = [1, \"two\", true, 4.0, nil]; print(type(mixed[4]));") == "SUCCESS");
    
    std::cout << "Edge cases tests completed.\n";
}

} // anonymous namespace
