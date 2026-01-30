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

// Stress test nested structures
TEST(StressLimits, DeeplyNestedArrays) {
    std::string code = R"(
        // Create deeply nested arrays
        level1 = [1];
        level2 = [level1];
        level3 = [level2];
        level4 = [level3];
        level5 = [level4];
        level6 = [level5];
        level7 = [level6];
        level8 = [level7];
        level9 = [level8];
        level10 = [level9];
        
        // Access deeply nested elements
        result = level10[0][0][0][0][0][0][0][0][0];
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("1") != std::string::npos);
}

TEST(StressLimits, MassiveArrayCreation) {
    std::string code = R"(
        // Create a large array
        bigArray = [];
        for (let i = 0; i < 100; i = i + 1) {
            bigArray.push(i);
        }
        print bigArray.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("100") != std::string::npos);
}

TEST(StressLimits, MassiveHashMapCreation) {
    std::string code = R"(
        // Create a large hash map
        bigMap = {};
        for (let i = 0; i < 50; i = i + 1) {
            key = "key" + str(i);
            bigMap[key] = i * 2;
        }
        print bigMap.size;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("50") != std::string::npos);
}

TEST(StressLimits, ComplexNestedStructures) {
    std::string code = R"(
        // Complex nested structure with arrays and hash maps
        complex = {
            "arrays": [
                [1, 2, 3],
                {"nested": [4, 5, {"deep": [6, 7]}]},
                [8, {"more": [9, 10]}]
            ],
            "maps": {
                "level1": {
                    "level2": {
                        "level3": [11, 12, 13]
                    }
                }
            }
        };
        
        result = complex.arrays[1].nested.deep[0];
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("6") != std::string::npos);
}

TEST(StressLimits, HeavyRecursion) {
    std::string code = R"(
        // Deep recursion test
        factorial = fun(n) {
            if (n <= 1) {
                return 1;
            } else {
                return n * factorial(n - 1);
            }
        };
        
        result = factorial(10);
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("3628800") != std::string::npos);
}

TEST(StressLimits, ClosureStressTest) {
    std::string code = R"(
        // Create many closures
        counters = [];
        
        for (let i = 0; i < 5; i = i + 1) {
            makeCounter = fun(initial) {
                count = initial;
                return fun() {
                    count = count + 1;
                    return count;
                };
            };
            
            counter = makeCounter(i * 10);
            counters.push(counter);
        }
        
        // Test each counter
        result1 = counters[0]();
        result2 = counters[1]();
        result3 = counters[2]();
        result4 = counters[3]();
        result5 = counters[4]();
        
        total = result1 + result2 + result3 + result4 + result5;
        print total;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Expected: (0*10+1) + (1*10+1) + (2*10+1) + (3*10+1) + (4*10+1) = 1 + 11 + 21 + 31 + 41 = 105
    EXPECT_TRUE(output.find("105") != std::string::npos);
}

TEST(StressLimits, FunctionalProgrammingStress) {
    std::string code = R"(
        // Chain multiple functional operations
        numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        
        // Filter evens, square them, then sum
        result = numbers
            .filter(fun(x) { return x % 2 == 0; })
            .map(fun(x) { return x * x; })
            .reduce(fun(acc, x) { return acc + x; }, 0);
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Evens: 2,4,6,8,10 -> squared: 4,16,36,64,100 -> sum: 220
    EXPECT_TRUE(output.find("220") != std::string::npos);
}

TEST(StressLimits, StringProcessingStress) {
    std::string code = R"(
        // Complex string operations
        text = "The quick brown fox jumps over the lazy dog";
        
        // Chain multiple string operations
        result = text
            .toLowerCase()
            .replace(" ", "_")
            .replace("_the_", "_THE_")
            .padStart(100, "-")
            .padEnd(120, "=");
        
        print len(result);
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("120") != std::string::npos);
}

TEST(StressLimits, FileOperationsStress) {
    std::string code = R"(
        // Test file operations
        content = "Line 1\nLine 2\nLine 3\nThis is a longer line with more content\nFinal line";
        
        // Write and read back
        success = writeFile("stress_test.txt", content);
        readBack = readFile("stress_test.txt");
        
        // Check if content matches
        matches = (len(readBack) == len(content));
        print matches;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(StressLimits, JSONComplexStructure) {
    std::string code = R"(
        // Complex JSON structure
        complexObj = {
            "users": [
                {
                    "id": 1,
                    "name": "Alice",
                    "profile": {
                        "age": 25,
                        "preferences": ["coding", "reading", "music"],
                        "settings": {
                            "theme": "dark",
                            "notifications": true
                        }
                    }
                },
                {
                    "id": 2,
                    "name": "Bob",
                    "profile": {
                        "age": 30,
                        "preferences": ["sports", "travel", "cooking"],
                        "settings": {
                            "theme": "light",
                            "notifications": false
                        }
                    }
                }
            ],
            "metadata": {
                "total": 2,
                "version": "1.0",
                "active": true
            }
        };
        
        // Encode and decode
        jsonStr = jsonEncode(complexObj);
        decoded = jsonDecode(jsonStr);
        
        // Verify structure
        firstUserId = decoded.users[0].id;
        secondUserName = decoded.users[1].name;
        
        print firstUserId;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("1") != std::string::npos);
}

TEST(StressLimits, PerformanceBenchmarkStress) {
    std::string code = R"(
        // Performance test with benchmark
        slowFunction = fun(n) {
            result = 0;
            for (let i = 0; i < n; i = i + 1) {
                for (let j = 0; j < n; j = j + 1) {
                    result = result + 1;
                }
            }
            return result;
        };
        
        // Benchmark the function
        benchResult = benchmark(slowFunction, 50);
        
        // Verify it ran and got reasonable result
        print benchResult.result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("2500") != std::string::npos); // 50 * 50
}

TEST(StressLimits, MathFunctionStress) {
    std::string code = R"(
        // Test multiple math functions together
        angle = 3.14159 / 4;  // 45 degrees in radians
        
        sinVal = sin(angle);
        cosVal = cos(angle);
        tanVal = tan(angle);
        hypotenuse = sqrt(sinVal * sinVal + cosVal * cosVal);
        
        // Logarithmic and exponential
        logVal = log(2.71828);  // Should be ~1
        expVal = exp(1);        // Should be ~2.71828
        
        // Combine results
        result = round(sinVal * 100) + round(cosVal * 100);
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // sin(π/4) ≈ 0.707, cos(π/4) ≈ 0.707, so rounded sum ≈ 71 + 71 = 142
    EXPECT_TRUE(output.find("142") != std::string::npos);
}

TEST(StressLimits, DateFunctionStress) {
    std::string code = R"(
        // Test date/time functions extensively
        timestamp = now();
        
        // Format in different ways
        fmt1 = formatDate(timestamp, "YYYY-MM-DD");
        fmt2 = formatDate(timestamp, "DD/MM/YYYY");
        fmt3 = formatDate(timestamp, "MM-DD-YYYY HH:mm:ss");
        
        // Test timestamp operations
        print len(str(timestamp)) > 10;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(StressLimits, LoopInterruptionStress) {
    std::string code = R"(
        // Test complex loop interruptions
        sum = 0;
        outer = 0;
        
        for (let i = 0; i < 10; i = i + 1) {
            for (let j = 0; j < 10; j = j + 1) {
                if (j == 5) break;  // Break inner loop
                if (i == 7) continue;  // Skip to next i
                sum = sum + 1;
            }
            outer = outer + 1;
            if (i == 8) break;  // Break outer loop
        }
        
        print sum;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Inner loop runs 8 times (0-7), each time adds 5 (0-4), so 8*5 = 40
    EXPECT_TRUE(output.find("40") != std::string::npos);
}

TEST(StressLimits, FunctionCompositionStress) {
    std::string code = R"(
        // Complex function composition
        addTen = fun(x) { return x + 10; };
        multiplyByTwo = fun(x) { return x * 2; };
        square = fun(x) { return x * x; };
        subtractOne = fun(x) { return x - 1; };
        
        // Compose multiple functions
        complexFunc = compose(subtractOne, square, multiplyByTwo, addTen);
        
        // Apply: addTen(5)=15, multiplyByTwo(15)=30, square(30)=900, subtractOne(900)=899
        result = complexFunc(5);
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("899") != std::string::npos);
}

TEST(StressLimits, ArrayMethodChainStress) {
    std::string code = R"(
        // Chain multiple array methods
        numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        
        result = numbers
            .filter(fun(x) { return x > 3; })      // [4, 5, 6, 7, 8, 9, 10]
            .map(fun(x) { return x * 2; })         // [8, 10, 12, 14, 16, 18, 20]
            .filter(fun(x) { return x % 3 == 0; }) // [12, 18]
            .reduce(fun(acc, x) { return acc + x; }, 0); // 30
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("30") != std::string::npos);
}

TEST(StressLimits, HashMapMethodChainStress) {
    std::string code = R"(
        // Complex hash map operations
        user = {
            "name": "Alice",
            "age": 25,
            "active": true,
            "scores": [85, 92, 78, 96]
        };
        
        // Add computed fields
        user.totalScore = user.scores.reduce(fun(acc, x) { return acc + x; }, 0);
        user.average = user.totalScore / user.scores.length;
        user.passed = user.average >= 80;
        
        print user.passed;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(StressLimits, MemoryAllocationStress) {
    std::string code = R"(
        // Create many objects to test memory management
        objects = [];
        
        for (let i = 0; i < 20; i = i + 1) {
            obj = {
                "id": i,
                "data": [i, i*2, i*3, i*4],
                "computed": i * i + i,
                "nested": {
                    "level": i,
                    "items": [i, i+1, i+2]
                }
            };
            objects.push(obj);
        }
        
        print objects.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("20") != std::string::npos);
}

TEST(StressLimits, EdgeCaseStress) {
    std::string code = R"(
        // Test edge cases and boundary conditions
        // Empty structures
        emptyArr = [];
        emptyMap = {};
        
        // Single element
        singleArr = [42];
        singleMap = { "key": "value" };
        
        // Extreme values
        bigNum = 999999999.0;
        smallNum = 0.000000001;
        
        // String extremes
        emptyStr = "";
        longStr = repeat("A", 100);
        
        // Verify operations work
        results = [
            emptyArr.length,
            emptyMap.size,
            singleArr[0],
            singleMap.keys().length,
            len(longStr)
        ];
        
        total = results.reduce(fun(acc, x) { return acc + x; }, 0);
        print total;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // 0 + 0 + 42 + 1 + 100 = 143
    EXPECT_TRUE(output.find("143") != std::string::npos);
}