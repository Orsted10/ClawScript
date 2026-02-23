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

// Performance and limit tests
TEST(PerformanceLimits, MassiveComputationTest) {
    std::string code = R"(
        // Perform massive computation
        result = 0;
        for (let i = 0; i < 1000; i = i + 1) {
            for (let j = 0; j < 100; j = j + 1) {
                result = result + (i * j);
            }
        }
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // The result should be the sum of i*j for i in 0..999 and j in 0..99
    // This is computationally intensive but should complete
}

TEST(PerformanceLimits, LargeStringOperations) {
    std::string code = R"(
        // Test large string operations
        base = "A";
        result = base;
        
        for (let i = 0; i < 10; i = i + 1) {
            result = result + result;  // Double the string each time
        }
        
        print len(result);
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // After 10 doublings of "A", we get 2^10 = 1024 characters
    EXPECT_TRUE(output.find("1024") != std::string::npos);
}

TEST(PerformanceLimits, DeepRecursionPerformance) {
    std::string code = R"(
        // Test deep but performant recursion
        fibonacci = fun(n) {
            if (n <= 1) {
                return n;
            }
            return fibonacci(n - 1) + fibonacci(n - 2);
        };
        
        // Calculate fibonacci of a moderate number
        result = fibonacci(15);
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Fibonacci of 15 is 610
    EXPECT_TRUE(output.find("610") != std::string::npos);
}

TEST(PerformanceLimits, LargeArraySort) {
    std::string code = R"(
        // Create and sort a large array
        arr = [];
        
        // Fill array with randomish values
        for (let i = 0; i < 50; i = i + 1) {
            arr.push((i * 7) % 50);
        }
        
        // Sort the array
        sorted = arr.sort(fun(a, b) { return a < b; });
        
        print sorted.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("50") != std::string::npos);
}

TEST(PerformanceLimits, HashMapLargeScale) {
    std::string code = R"(
        // Test hash map with many entries
        map = {};
        
        for (let i = 0; i < 100; i = i + 1) {
            key = "key" + str(i);
            map[key] = i * i;  // Store squares
        }
        
        print map.size;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("100") != std::string::npos);
}

TEST(PerformanceLimits, FunctionCallPerformance) {
    std::string code = R"(
        // Test performance of function calls
        simpleFunc = fun(x) {
            return x + 1;
        };
        
        result = 0;
        for (let i = 0; i < 1000; i = i + 1) {
            result = simpleFunc(result);
        }
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("1000") != std::string::npos);
}

TEST(PerformanceLimits, NestedLoopPerformance) {
    std::string code = R"(
        // Test nested loop performance
        count = 0;
        
        for (let i = 0; i < 20; i = i + 1) {
            for (let j = 0; j < 20; j = j + 1) {
                for (let k = 0; k < 20; k = k + 1) {
                    count = count + 1;
                }
            }
        }
        
        print count;  // Should be 20 * 20 * 20 = 8000
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("8000") != std::string::npos);
}

TEST(PerformanceLimits, StringConcatenationPerformance) {
    std::string code = R"(
        // Test string concatenation performance
        result = "";
        
        for (let i = 0; i < 100; i = i + 1) {
            result = result + str(i) + "-";
        }
        
        print len(result);
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Result will be "0-1-2-3-...99-" which has more than 100 characters
    EXPECT_TRUE(output.find("200") != std::string::npos || output.find("300") != std::string::npos);
}

TEST(PerformanceLimits, ArrayMethodChainingPerformance) {
    std::string code = R"(
        // Test performance of chained array methods
        numbers = [];
        
        for (let i = 0; i < 100; i = i + 1) {
            numbers.push(i);
        }
        
        result = numbers
            .filter(fun(x) { return x % 2 == 0; })  // Even numbers
            .map(fun(x) { return x * 2; })          // Double them
            .slice(0, 10);                          // Take first 10
        
        print result.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("10") != std::string::npos);
}

TEST(PerformanceLimits, ClosureCreationPerformance) {
    std::string code = R"(
        // Test performance of creating many closures
        closures = [];
        
        for (let i = 0; i < 50; i = i + 1) {
            makeClosure = fun(value) {
                captured = value;
                return fun() {
                    return captured * 2;
                };
            };
            
            closure = makeClosure(i);
            closures.push(closure);
        }
        
        // Execute a few closures to test they work
        sum = 0;
        for (let i = 0; i < 10; i = i + 1) {
            sum = sum + closures[i]();
        }
        
        print sum;  // Should be 0*2 + 1*2 + ... + 9*2 = 90
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("90") != std::string::npos);
}

TEST(PerformanceLimits, DeepObjectCreation) {
    std::string code = R"(
        // Test creation of deep object structures
        createDeepObject = fun(depth) {
            if (depth <= 0) {
                return { "value": 42 };
            }
            return {
                "level": depth,
                "child": createDeepObject(depth - 1)
            };
        };
        
        deepObj = createDeepObject(10);
        print deepObj.level;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("10") != std::string::npos);
}

TEST(PerformanceLimits, MathematicalFunctionPerformance) {
    std::string code = R"(
        // Test performance of mathematical functions
        result = 0;
        
        for (let i = 1; i <= 50; i = i + 1) {
            result = result + sin(float(i)) + cos(float(i)) + tan(float(i) / 10);
        }
        
        print round(result * 100) / 100;  // Round to 2 decimal places
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
}

TEST(PerformanceLimits, FileOperationPerformance) {
    std::string code = R"(
        // Test file operations performance
        // Create multiple small files
        for (let i = 0; i < 10; i = i + 1) {
            filename = "perf_test_" + str(i) + ".txt";
            content = "Performance test file " + str(i) + "\nCreated for performance testing";
            success = writeFile(filename, content);
        }
        
        // Read them back
        totalLength = 0;
        for (let i = 0; i < 10; i = i + 1) {
            filename = "perf_test_" + str(i) + ".txt";
            content = readFile(filename);
            totalLength = totalLength + len(content);
        }
        
        print totalLength;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    // Each file has ~40+ characters, 10 files, so result should be > 400
    EXPECT_TRUE(std::stoi(output) > 400);
}

TEST(PerformanceLimits, JSONSerializationPerformance) {
    std::string code = R"(
        // Test JSON serialization performance with complex objects
        complexObj = {
            "arrays": [
                [1, 2, 3, 4, 5],
                [6, 7, 8, 9, 10],
                [11, 12, 13, 14, 15]
            ],
            "objects": {
                "nested1": { "values": [100, 200, 300] },
                "nested2": { "values": [400, 500, 600] }
            },
            "mixed": [
                "string",
                42,
                true,
                nil,
                { "inner": "object" }
            ]
        };
        
        // Serialize and deserialize multiple times
        for (let i = 0; i < 5; i = i + 1) {
            jsonStr = jsonEncode(complexObj);
            parsed = jsonDecode(jsonStr);
        }
        
        print len(jsonStr) > 100;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(PerformanceLimits, StringMethodPerformance) {
    std::string code = R"(
        // Test performance of string methods
        text = "The quick brown fox jumps over the lazy dog. ";
        
        // Repeat the text to make it larger
        for (let i = 0; i < 5; i = i + 1) {
            text = text + text;
        }
        
        // Apply multiple string operations
        result = text
            .toLowerCase()
            .replace("the", "THE")
            .replace("quick", "QUICK")
            .replace("brown", "BROWN")
            .padEnd(len(text) + 20, "X")
            .substring(0, 100);
        
        print len(result);
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("100") != std::string::npos);
}

TEST(PerformanceLimits, HashmapMethodPerformance) {
    std::string code = R"(
        // Test performance of hashmap methods
        map = {};
        
        // Fill with data
        for (let i = 0; i < 50; i = i + 1) {
            map[str(i)] = i * i;
        }
        
        // Test various methods
        keys = map.keys();
        values = map.values();
        size = map.size;
        
        // Test has and get operations in a loop
        count = 0;
        for (let i = 0; i < 25; i = i + 1) {
            if (map.has(str(i))) {
                count = count + 1;
            }
        }
        
        print count;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("25") != std::string::npos);
}

TEST(PerformanceLimits, ArrayMethodPerformance) {
    std::string code = R"(
        // Test performance of array methods
        arr = [];
        
        for (let i = 0; i < 100; i = i + 1) {
            arr.push(i);
        }
        
        // Chain multiple operations
        result = arr
            .slice(10, 30)              // Get elements 10-29
            .filter(fun(x) { return x % 2 == 0; })  // Even numbers only
            .map(fun(x) { return x * 3; })          // Triple them
            .reverse()                  // Reverse the order
            .concat([999, 888]);        // Add extra elements
        
        print result.length;  // Should be 10 (even numbers from 10-28) + 2 = 12
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("12") != std::string::npos);
}

TEST(PerformanceLimits, FunctionalPipelinePerformance) {
    std::string code = R"(
        // Test performance of functional programming pipeline
        numbers = [];
        
        for (let i = 0; i < 200; i = i + 1) {
            numbers.push(i);
        }
        
        result = numbers
            .filter(fun(x) { return x > 50 && x < 150; })     // 51-149 = 99 numbers
            .map(fun(x) { return x * x; })                    // Square them
            .filter(fun(x) { return x % 3 == 0; })            // Divisible by 3
            .map(fun(x) { return sqrt(float(x)); })           // Back to original (approximately)
            .reduce(fun(acc, x) { return acc + int(x); }, 0); // Sum them
        
        print result > 0;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(PerformanceLimits, MemoryAllocationStress) {
    std::string code = R"(
        // Stress test memory allocation
        containers = [];
        
        for (let i = 0; i < 30; i = i + 1) {
            container = {
                "id": i,
                "data": [],
                "metadata": { "created": i * 100 }
            };
            
            // Fill data array
            for (let j = 0; j < 10; j = j + 1) {
                container.data.push({
                    "index": j,
                    "value": i * j,
                    "nested": [i, j, i+j]
                });
            }
            
            containers.push(container);
        }
        
        print containers.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("30") != std::string::npos);
}

TEST(PerformanceLimits, ComplexBenchmarking) {
    std::string code = R"(
        // Complex benchmark scenario
        complexOperation = fun(size) {
            result = 0;
            data = [];
            
            // Create data
            for (let i = 0; i < size; i = i + 1) {
                data.push({
                    "value": i,
                    "computed": i * i + i,
                    "flag": i % 3 == 0
                });
            }
            
            // Process data
            filtered = data.filter(fun(item) { return item.flag; });
            mapped = filtered.map(fun(item) { return item.computed; });
            result = mapped.reduce(fun(acc, val) { return acc + val; }, 0);
            
            return result;
        };
        
        // Benchmark the operation
        startTime = clock();
        result = complexOperation(fastCount(100));
        endTime = clock();
        duration = endTime - startTime;
        
        print result > 0;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(PerformanceLimits, DateFunctionPerformance) {
    std::string code = R"(
        // Test performance of date functions
        timestamps = [];
        
        // Create multiple timestamps
        for (let i = 0; i < 10; i = i + 1) {
            timestamps.push(now());
            // Small delay simulation
        }
        
        // Format them
        formatted = [];
        for (let i = 0; i < timestamps.length; i = i + 1) {
            formatted.push(formatDate(timestamps[i], "YYYY-MM-DD HH:mm:ss"));
        }
        
        print timestamps.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("10") != std::string::npos);
}

TEST(PerformanceLimits, NestedFunctionCalls) {
    std::string code = R"(
        // Test performance of nested function calls
        deepFunction = fun(level) {
            if (level <= 0) {
                return 1;
            }
            return 1 + deepFunction(level - 1);
        };
        
        result = 0;
        for (let i = 0; i < 10; i = i + 1) {
            result = result + deepFunction(5);  // Should return 6 each time (5+1)
        }
        
        print result;  // Should be 10 * 6 = 60
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("60") != std::string::npos);
}

TEST(PerformanceLimits, LargeNumericComputations) {
    std::string code = R"(
        // Test large numeric computations
        result = 1.0;
        
        for (let i = 1; i <= 20; i = i + 1) {
            result = result * float(i);  // Compute factorial of 20
        }
        
        print result > 1000000.0;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(PerformanceLimits, MixedDataStructureOperations) {
    std::string code = R"(
        // Test operations mixing arrays and hashmaps
        mixedData = [];
        
        for (let i = 0; i < 25; i = i + 1) {
            item = {
                "id": i,
                "tags": ["tag" + str(i), "common"],
                "stats": {
                    "count": i * 2,
                    "active": i % 2 == 0
                }
            };
            mixedData.push(item);
        }
        
        // Process mixed data
        activeItems = mixedData.filter(fun(item) { return item.stats.active; });
        tagCounts = {};
        
        for (let i = 0; i < activeItems.length; i = i + 1) {
            tags = activeItems[i].tags;
            for (let j = 0; j < tags.length; j = j + 1) {
                tag = tags[j];
                if (tagCounts.has(tag)) {
                    tagCounts[tag] = tagCounts[tag] + 1;
                } else {
                    tagCounts[tag] = 1;
                }
            }
        }
        
        print activeItems.length;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("13") != std::string::npos);  // 0, 2, 4, ..., 24 = 13 items
}

TEST(PerformanceLimits, FunctionalReducePerformance) {
    std::string code = R"(
        // Test performance of reduce operations
        largeArray = [];
        
        for (let i = 0; i < 500; i = i + 1) {
            largeArray.push(i);
        }
        
        // Multiple reductions
        sum = largeArray.reduce(fun(acc, x) { return acc + x; }, 0);
        productSmall = largeArray.slice(0, 10).reduce(fun(acc, x) { return acc * (x + 1); }, 1);
        max = largeArray.reduce(fun(acc, x) { return x > acc ? x : acc; }, largeArray[0]);
        
        print sum > 10000;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}
