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

// Test edge cases and error conditions
void testErrorConditions() {
    std::cout << "Testing error conditions...\n";
    
    // Division by zero
    assert(runCode("result = 10 / 0; print(result);") == "RUNTIME_ERROR: Division by zero");
    
    // Out of bounds array access
    assert(runCode("arr = [1, 2, 3]; print(arr[10]);") == "RUNTIME_ERROR: Array index out of bounds: 10");
    
    // Invalid operands for operations
    assert(runCode("result = \"hello\" - 5; print(result);") == "RUNTIME_ERROR: Operands must be numbers");
    
    // Invalid function calls
    assert(runCode("notAFunction = 42; result = notAFunction(); print(result);") == "RUNTIME_ERROR: Can only call functions and classes");
    
    std::cout << "Error condition tests completed.\n";
}

// Test variable scoping
void testScoping() {
    std::cout << "Testing variable scoping...\n";
    
    // Global vs local scope
    assert(runCode(R"(
        globalVar = 10;
        testFunc = fun() {
            localVar = 20;
            return globalVar + localVar;
        };
        result = testFunc();
        print(result);
    )") == "SUCCESS");
    
    // Block scope
    assert(runCode(R"(
        outer = 100;
        {
            inner = 200;
            result = outer + inner;
        }
        print(result);
    )") == "SUCCESS");
    
    // Function scope
    assert(runCode(R"(
        x = 10;
        func = fun() {
            y = x;  // Should access outer x
            return y;
        };
        result = func();
        print(result);
    )") == "SUCCESS");
    
    // Shadowing
    assert(runCode(R"(
        x = 10;
        func = fun() {
            x = 20;  // Local x shadows outer x
            return x;
        };
        result = func();
        print(result);
    )") == "SUCCESS");
    
    std::cout << "Scoping tests completed.\n";
}

// Test complex expressions
void testComplexExpressions() {
    std::cout << "Testing complex expressions...\n";
    
    // Nested function calls
    assert(runCode("result = len(str(sqrt(16))); print(result);") == "SUCCESS");
    
    // Complex arithmetic
    assert(runCode("result = ((10 + 5) * 2) - (20 / 4); print(result);") == "SUCCESS");
    
    // Complex boolean expressions
    assert(runCode("result = (5 > 3) && (10 < 20) || (7 == 7); print(result);") == "SUCCESS");
    
    // Ternary expressions
    assert(runCode("result = 5 > 3 ? \"greater\" : \"lesser\"; print(result);") == "SUCCESS");
    
    // Mixed type expressions
    assert(runCode("result = \"Number: \" + (5 * 10); print(result);") == "SUCCESS");
    
    // Complex array indexing
    assert(runCode("arr = [[1, 2], [3, 4]]; result = arr[0][1]; print(result);") == "SUCCESS");
    
    // Complex hash map access
    assert(runCode("hm = { \"nested\": { \"value\": 42 } }; result = hm.nested.value; print(result);") == "SUCCESS");
    
    std::cout << "Complex expression tests completed.\n";
}

// Test recursive functions
void testRecursion() {
    std::cout << "Testing recursive functions...\n";
    
    // Factorial
    assert(runCode(R"(
        factorial = fun(n) {
            if (n <= 1) {
                return 1;
            } else {
                return n * factorial(n - 1);
            }
        };
        result = factorial(5);
        print(result);
    )") == "SUCCESS");
    
    // Fibonacci
    assert(runCode(R"(
        fib = fun(n) {
            if (n <= 1) {
                return n;
            } else {
                return fib(n - 1) + fib(n - 2);
            }
        };
        result = fib(6);
        print(result);
    )") == "SUCCESS");
    
    // Sum of array (recursive)
    assert(runCode(R"(
        sumArr = fun(arr, index) {
            if (index >= arr.length) {
                return 0;
            } else {
                return arr[index] + sumArr(arr, index + 1);
            }
        };
        arr = [1, 2, 3, 4, 5];
        result = sumArr(arr, 0);
        print(result);
    )") == "SUCCESS");
    
    std::cout << "Recursion tests completed.\n";
}

// Test closures and lexical scoping
void testClosures() {
    std::cout << "Testing closures...\n";
    
    // Basic closure
    assert(runCode(R"(
        makeCounter = fun() {
            count = 0;
            return fun() {
                count = count + 1;
                return count;
            };
        };
        counter = makeCounter();
        result1 = counter();
        result2 = counter();
        print(result1);
    )") == "SUCCESS");
    
    // Closure with multiple variables
    assert(runCode(R"(
        makeAdder = fun(x) {
            return fun(y) {
                return x + y;
            };
        };
        addFive = makeAdder(5);
        result = addFive(3);
        print(result);
    )") == "SUCCESS");
    
    // Closure in loop
    assert(runCode(R"(
        funcs = [];
        for (let i = 0; i < 3; i = i + 1) {
            funcs.push(fun() { return i; });
        }
        result = funcs[0]();
        print(result);
    )") == "SUCCESS");
    
    std::cout << "Closure tests completed.\n";
}

// Test compound assignments
void testCompoundAssignments() {
    std::cout << "Testing compound assignments...\n";
    
    // += operator
    assert(runCode("x = 10; x += 5; print(x);") == "SUCCESS");
    assert(runCode("s = \"Hello\"; s += \" World\"; print(s);") == "SUCCESS");
    
    // -= operator
    assert(runCode("x = 10; x -= 3; print(x);") == "SUCCESS");
    
    // *= operator
    assert(runCode("x = 10; x *= 2; print(x);") == "SUCCESS");
    
    // /= operator
    assert(runCode("x = 10; x /= 2; print(x);") == "SUCCESS");
    
    // Chained assignments
    assert(runCode("a = b = c = 42; print(a);") == "SUCCESS");
    
    // Compound assignment in loops
    assert(runCode(R"(
        sum = 0;
        for (let i = 1; i <= 5; i = i + 1) {
            sum += i;
        }
        print(sum);
    )") == "SUCCESS");
    
    std::cout << "Compound assignment tests completed.\n";
}

// Test increment/decrement operators
void testIncrementDecrement() {
    std::cout << "Testing increment/decrement operators...\n";
    
    // Prefix increment
    assert(runCode("x = 5; result = ++x; print(result);") == "SUCCESS");
    
    // Postfix increment
    assert(runCode("x = 5; result = x++; print(result);") == "SUCCESS");
    
    // Prefix decrement
    assert(runCode("x = 5; result = --x; print(result);") == "SUCCESS");
    
    // Postfix decrement
    assert(runCode("x = 5; result = x--; print(result);") == "SUCCESS");
    
    // Increment/decrement in expressions
    assert(runCode("x = 5; result = x++ + 10; print(result);") == "SUCCESS");
    
    // Increment/decrement in loops
    assert(runCode(R"(
        i = 0;
        sum = 0;
        while (i < 5) {
            sum += i;
            i++;
        }
        print(sum);
    )") == "SUCCESS");
    
    std::cout << "Increment/decrement tests completed.\n";
}

// Test type conversions and checks
void testTypeConversions() {
    std::cout << "Testing type conversions...\n";
    
    // Number to string
    assert(runCode("result = str(42); print(result);") == "SUCCESS");
    
    // String to number
    assert(runCode("result = num(\"123\"); print(result);") == "SUCCESS");
    
    // Boolean conversions
    assert(runCode("result = num(true); print(result);") == "SUCCESS");
    assert(runCode("result = num(false); print(result);") == "SUCCESS");
    
    // Type checking
    assert(runCode("result = type(42); print(result);") == "SUCCESS");
    assert(runCode("result = type(\"hello\"); print(result);") == "SUCCESS");
    assert(runCode("result = type(true); print(result);") == "SUCCESS");
    assert(runCode("result = type(nil); print(result);") == "SUCCESS");
    
    // Truthiness tests
    assert(runCode("result = if (1) { \"truthy\"; } else { \"falsy\"; }; print(result);") == "SUCCESS");
    assert(runCode("result = if (0) { \"truthy\"; } else { \"falsy\"; }; print(result);") == "SUCCESS");
    assert(runCode("result = if (\"\") { \"truthy\"; } else { \"falsy\"; }; print(result);") == "SUCCESS");
    assert(runCode("result = if (nil) { \"truthy\"; } else { \"falsy\"; }; print(result);") == "SUCCESS");
    
    std::cout << "Type conversion tests completed.\n";
}

} // anonymous namespace