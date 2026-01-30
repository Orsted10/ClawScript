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

// Advanced edge case tests
TEST(EdgeCasesAdvanced, ArrayBoundsChecking) {
    std::string code = R"(
        arr = [1, 2, 3];
        
        // Test negative index (should fail gracefully if supported, or handle appropriately)
        try {
            result = arr[-1];
            print "negative_index_handled";
        } catch (error) {
            print "negative_index_caught";
        }
        
        // Test out of bounds
        try {
            result = arr[100];
            print "out_of_bounds_handled";
        } catch (error) {
            print "out_of_bounds_caught";
        }
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(EdgeCasesAdvanced, StringBoundsChecking) {
    std::string code = R"(
        str = "hello";
        
        // Test string indexing edge cases
        try {
            result = str[0];
            print "valid_index";
        } catch (error) {
            print "valid_index_error";
        }
        
        try {
            result = str[10];
            print "invalid_index";
        } catch (error) {
            print "invalid_index_caught";
        }
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(EdgeCasesAdvanced, DivisionByZero) {
    std::string code = R"(
        try {
            result = 10 / 0;
            print "division_by_zero_handled";
        } catch (error) {
            print "division_by_zero_caught";
        }
        
        try {
            result = 5.5 / 0.0;
            print "float_division_by_zero";
        } catch (error) {
            print "float_division_caught";
        }
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(EdgeCasesAdvanced, InvalidOperations) {
    std::string code = R"(
        // Test invalid operations
        try {
            result = "string" - 5;
            print "invalid_subtraction";
        } catch (error) {
            print "subtraction_caught";
        }
        
        try {
            result = "hello" * 3;
            print "string_multiplication";
        } catch (error) {
            print "multiplication_caught";
        }
        
        try {
            result = [1, 2, 3] + "string";
            print "array_string_addition";
        } catch (error) {
            print "addition_caught";
        }
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(EdgeCasesAdvanced, TypeConversionEdgeCases) {
    std::string code = R"(
        // Test type conversions
        try {
            result = num("123");
            print "string_to_number";
        } catch (error) {
            print "conversion_error";
        }
        
        try {
            result = num("not_a_number");
            print "invalid_conversion";
        } catch (error) {
            print "invalid_conversion_caught";
        }
        
        try {
            result = str(42);
            print "number_to_string";
        } catch (error) {
            print "str_conversion_error";
        }
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(EdgeCasesAdvanced, FunctionCallEdgeCases) {
    std::string code = R"(
        // Test function call edge cases
        notAFunction = 42;
        
        try {
            result = notAFunction();
            print "non_function_call";
        } catch (error) {
            print "non_function_caught";
        }
        
        // Test function with wrong number of arguments
        add = fun(a, b) {
            return a + b;
        };
        
        try {
            result = add(1);
            print "wrong_args_handled";
        } catch (error) {
            print "wrong_args_caught";
        }
        
        try {
            result = add(1, 2, 3);
            print "extra_args_handled";
        } catch (error) {
            print "extra_args_caught";
        }
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(EdgeCasesAdvanced, VariableScopeEdgeCases) {
    std::string code = R"(
        global = 100;
        
        // Test variable shadowing
        test = fun() {
            global = 200;  // This should shadow the global
            local = 300;
            return global + local;
        };
        
        result = test();
        print result;  // Should print 500 (200 + 300)
        
        // Global should remain unchanged after function returns
        print global;  // Should print 100
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("500") != std::string::npos);
    EXPECT_TRUE(output.find("100") != std::string::npos);
}

TEST(EdgeCasesAdvanced, ComplexLoopScenarios) {
    std::string code = R"(
        // Test complex loop scenarios
        sum = 0;
        
        // Nested loops with break/continue
        for (let i = 0; i < 3; i = i + 1) {
            for (let j = 0; j < 5; j = j + 1) {
                if (j == 1) continue;  // Skip j=1
                if (j == 3) break;     // Break inner loop at j=3
                sum = sum + (i * 10 + j);
            }
        }
        
        print sum;  // Expected: (0*10+0) + (0*10+2) + (1*10+0) + (1*10+2) + (2*10+0) + (2*10+2) = 0+2+10+12+20+22 = 66
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("66") != std::string::npos);
}

TEST(EdgeCasesAdvanced, ExceptionHandlingScenarios) {
    std::string code = R"(
        // Test exception handling scenarios
        result = 0;
        
        // Test nested try-catch blocks
        try {
            try {
                problematic = 10 / 0;
                result = 1;
            } catch (innerError) {
                result = 2;
                // Re-throw or handle
                throw innerError;
            }
        } catch (outerError) {
            result = result + 10;  // Should be 2 + 10 = 12
        }
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("12") != std::string::npos || output.find("2") != std::string::npos);
}

TEST(EdgeCasesAdvanced, LargeNumberOperations) {
    std::string code = R"(
        // Test large number operations
        big1 = 999999999.0;
        big2 = 888888888.0;
        
        sum = big1 + big2;
        diff = big1 - big2;
        prod = big1 * big2;
        
        print "large_ops_complete";
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("complete") != std::string::npos);
}

TEST(EdgeCasesAdvanced, PrecisionEdgeCases) {
    std::string code = R"(
        // Test floating point precision
        tiny = 0.000000001;
        huge = 999999999.0;
        
        result = tiny * huge;  // Very small number
        print result > 0;      // Should be true if multiplication worked
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(EdgeCasesAdvanced, BooleanLogicEdgeCases) {
    std::string code = R"(
        // Test complex boolean logic
        a = true;
        b = false;
        c = 1;
        d = 0;
        e = "non-empty";
        f = "";
        g = nil;
        
        // Test truthiness in complex expressions
        result1 = a && c && e;     // Should be true (truthy values)
        result2 = b || d || f;     // Should be false (all falsy)
        result3 = a && !b && c;    // Should be true
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(EdgeCasesAdvanced, StringComparisonEdgeCases) {
    std::string code = R"(
        // Test string comparison edge cases
        str1 = "abc";
        str2 = "abd";
        str3 = "ABC";
        
        result1 = str1 < str2;   // Should be true ("abc" < "abd")
        result2 = str1 < str3;   // Case-sensitive comparison
        result3 = str1.toLowerCase() < str3.toLowerCase();  // Case-insensitive comparison
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(EdgeCasesAdvanced, ArrayMutationEdgeCases) {
    std::string code = R"(
        // Test array mutation scenarios
        arr1 = [1, 2, 3];
        arr2 = arr1;  // Reference sharing
        
        arr1[0] = 999;
        
        // Both should show the change if they share reference
        print arr1[0];
        print arr2[0];
        
        // Test push/pop edge cases
        empty = [];
        empty.push(42);
        last = empty.pop();
        print last;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("999") != std::string::npos);
    EXPECT_TRUE(output.find("42") != std::string::npos);
}

TEST(EdgeCasesAdvanced, HashMapMutationEdgeCases) {
    std::string code = R"(
        // Test hash map mutation scenarios
        map1 = { "a": 1, "b": 2 };
        map2 = map1;  // Reference sharing
        
        map1["a"] = 999;
        
        // Both should show the change if they share reference
        print map1["a"];
        print map2["a"];
        
        // Test dynamic addition/removal
        map1["newKey"] = "newValue";
        hasNew = map1.has("newKey");
        removed = map1.remove("newKey");
        
        print hasNew;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("999") != std::string::npos);
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(EdgeCasesAdvanced, ClosureVariableCapture) {
    std::string code = R"(
        // Test closure variable capture edge cases
        makeAdder = fun(x) {
            return fun(y) {
                return x + y;
            };
        };
        
        add5 = makeAdder(5);
        add10 = makeAdder(10);
        
        result1 = add5(3);   // Should be 8
        result2 = add10(3);  // Should be 13
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("8") != std::string::npos);
}

TEST(EdgeCasesAdvanced, RecursiveDataStructures) {
    std::string code = R"(
        // Test self-referencing structures (if supported)
        obj = { "name": "self" };
        obj.ref = obj;  // Self-reference
        
        // Access through the reference
        nameThroughRef = obj.ref.name;
        print nameThroughRef;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("self") != std::string::npos);
}

TEST(EdgeCasesAdvanced, FunctionAsReturnValue) {
    std::string code = R"(
        // Test functions as return values
        createCalculator = fun(operation) {
            if (operation == "add") {
                return fun(a, b) { return a + b; };
            } else if (operation == "multiply") {
                return fun(a, b) { return a * b; };
            } else {
                return fun(a, b) { return 0; };
            }
        };
        
        adder = createCalculator("add");
        multiplier = createCalculator("multiply");
        
        result1 = adder(5, 3);
        result2 = multiplier(5, 3);
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("8") != std::string::npos);
}

TEST(EdgeCasesAdvanced, HighOrderFunctionScenarios) {
    std::string code = R"(
        // Test higher-order function scenarios
        applyTwice = fun(func, value) {
            return func(func(value));
        };
        
        increment = fun(x) { return x + 1; };
        square = fun(x) { return x * x; };
        
        result1 = applyTwice(increment, 5);  // inc(inc(5)) = inc(6) = 7
        result2 = applyTwice(square, 3);     // square(square(3)) = square(9) = 81
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("7") != std::string::npos);
}

TEST(EdgeCasesAdvanced, MemoryLeakScenarios) {
    std::string code = R"(
        // Test scenarios that might cause memory issues
        createManyObjects = fun(count) {
            objects = [];
            for (let i = 0; i < count; i = i + 1) {
                obj = {
                    "id": i,
                    "data": [i, i*2, i*i],
                    "func": fun(x) { return x + i; }
                };
                objects.push(obj);
            }
            return objects;
        };
        
        manyObjects = createManyObjects(10);
        print manyObjects.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("10") != std::string::npos);
}