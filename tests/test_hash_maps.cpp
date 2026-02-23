#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
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

// ==================== BASIC HASH MAP TESTS ====================

TEST(HashMap, EmptyHashMap) {
    std::string code = R"(
        let empty = {};
        print type(empty);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hashmap\n");
}

TEST(HashMap, BasicHashMapCreation) {
    std::string code = R"(
        let person = {"name": "Alice", "age": 25};
        print person["name"];
        print person["age"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "Alice\n25\n");
}

TEST(HashMap, MixedValueTypes) {
    std::string code = R"(
        let mixed = {
            "str": "hello",
            "num": 42,
            "bool": true,
            "nil": nil
        };
        print mixed["str"];
        print mixed["num"];
        print mixed["bool"];
        print mixed["nil"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hello\n42\ntrue\nnil\n");
}

TEST(HashMap, NumberKeys) {
    std::string code = R"(
        let nums = {1: "one", 2: "two", 3.5: "three_point_five"};
        print nums["1"];
        print nums["2"];
        print nums["3.5"];
    )";
    
    std::string output = runCode(code);
    // Just checking that the decimal key is preserved properly
    EXPECT_EQ(output, "one\ntwo\nthree_point_five\n");
}

// ==================== HASH MAP ACCESS TESTS ====================

TEST(HashMap, AccessNonexistentKey) {
    std::string code = R"(
        let map = {"exists": "yes"};
        print map["doesnt_exist"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "nil\n");
}

TEST(HashMap, AccessWithVariable) {
    std::string code = R"(
        let map = {"key1": "value1", "key2": "value2"};
        let key = "key1";
        print map[key];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "value1\n");
}

// ==================== HASH MAP ASSIGNMENT TESTS ====================

TEST(HashMap, Assignment) {
    std::string code = R"(
        let map = {};
        map["key"] = "new_value";
        print map["key"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "new_value\n");
}

TEST(HashMap, Reassignment) {
    std::string code = R"(
        let map = {"key": "old_value"};
        map["key"] = "new_value";
        print map["key"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "new_value\n");
}

TEST(HashMap, AssignmentWithVariables) {
    std::string code = R"(
        let map = {};
        let key = "dynamic_key";
        let value = "dynamic_value";
        map[key] = value;
        print map[key];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "dynamic_value\n");
}

// ==================== BUILT-IN FUNCTIONS TESTS ====================

TEST(HashMap, KeysFunction) {
    std::string code = R"(
        let map = {"a": 1, "b": 2, "c": 3};
        let keys = keys(map);
        print keys.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(HashMap, ValuesFunction) {
    std::string code = R"(
        let map = {"a": 10, "b": 20, "c": 30};
        let vals = values(map);
        print vals.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(HashMap, TypeFunction) {
    std::string code = R"(
        let map = {"key": "value"};
        print type(map);
        print type({});
        print type(keys(map));
        print type(values(map));
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "hashmap\nhashmap\narray\narray\n");
}

// ==================== MEMBER ACCESS TESTS ====================

TEST(HashMap, SizeMember) {
    std::string code = R"(
        let empty = {};
        let small = {"a": 1};
        let large = {"a": 1, "b": 2, "c": 3};
        print empty.size;
        print small.size;
        print large.size;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n3\n");
}

TEST(HashMap, KeysMember) {
    std::string code = R"(
        let map = {"x": 1, "y": 2};
        let mapKeys = map.keys();
        print mapKeys.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(HashMap, ValuesMember) {
    std::string code = R"(
        let map = {"x": 10, "y": 20};
        let mapVals = map.values();
        print mapVals.length;
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

// ==================== NESTED STRUCTURES TESTS ====================

TEST(HashMap, NestedHashMaps) {
    std::string code = R"(
        let nested = {
            "level1": {
                "level2": {"value": "nested"}
            }
        };
        print nested["level1"]["level2"]["value"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "nested\n");
}

TEST(HashMap, HashMapsWithArrays) {
    std::string code = R"(
        let map = {
            "arr": [1, 2, 3],
            "name": "test"
        };
        print map["arr"].length;
        print map["name"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\ntest\n");
}

TEST(HashMap, ArraysWithHashMaps) {
    std::string code = R"(
        let arr = [
            {"name": "Alice"},
            {"name": "Bob"}
        ];
        print arr[0]["name"];
        print arr[1]["name"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "Alice\nBob\n");
}

// ==================== TRUTHINESS AND EQUALITY TESTS ====================

TEST(HashMap, EmptyHashMapTruthiness) {
    std::string code = R"(
        let empty = {};
        if (empty) {
            print "truthy";
        } else {
            print "falsy";
        }
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "falsy\n");
}

TEST(HashMap, NonEmptyHashMapTruthiness) {
    std::string code = R"(
        let map = {"key": "value"};
        if (map) {
            print "truthy";
        } else {
            print "falsy";
        }
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "truthy\n");
}

// ==================== PRACTICAL EXAMPLES ====================

TEST(HashMap, StudentRecord) {
    std::string code = R"(
        let student = {
            "name": "John Doe",
            "grade": 85,
            "subjects": ["Math", "Science", "English"],
            "active": true
        };
        
        print student["name"];
        print student["grade"];
        print student["active"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "John Doe\n85\ntrue\n");
}

TEST(HashMap, ConfigurationObject) {
    std::string code = R"(
        let config = {
            "debug": true,
            "port": 8080,
            "host": "localhost",
            "features": {
                "auth": true,
                "logging": false
            }
        };
        
        print config["debug"];
        print config["port"];
        print config["features"]["auth"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "true\n8080\ntrue\n");
}

TEST(HashMap, CounterPattern) {
    std::string code = R"(
        let counts = {};
        counts["a"] = 0;
        counts["b"] = 0;
        counts["c"] = 0;
        
        counts["a"] = counts["a"] + 1;
        counts["a"] = counts["a"] + 1;
        counts["b"] = counts["b"] + 1;
        
        print counts["a"];
        print counts["b"];
        print counts["c"];
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n1\n0\n");
}

TEST(HashMap, CompoundMemberPlus) {
    std::string code = R"(
        let m = {"a": 1};
        m.a += 2;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(HashMap, CompoundMemberMinus) {
    std::string code = R"(
        let m = {"a": 10};
        m.a -= 4;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(HashMap, CompoundMemberMultiply) {
    std::string code = R"(
        let m = {"a": 7};
        m.a *= 3;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "21\n");
}

TEST(HashMap, CompoundMemberDivide) {
    std::string code = R"(
        let m = {"a": 18};
        m.a /= 6;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(HashMap, CompoundMemberXor) {
    std::string code = R"(
        let m = {"a": 5};
        m.a ^= 3;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(HashMap, CompoundMemberShiftLeft) {
    std::string code = R"(
        let m = {"a": 2};
        m.a <<= 3;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(HashMap, CompoundMemberShiftRight) {
    std::string code = R"(
        let m = {"a": 16};
        m.a >>= 2;
        print m.a;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(HashMap, CompoundIndexBitOr) {
    std::string code = R"(
        let m = {"k": 2};
        m["k"] |= 1;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(HashMap, CompoundIndexMinus) {
    std::string code = R"(
        let m = {"k": 10};
        m["k"] -= 4;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(HashMap, CompoundIndexMultiply) {
    std::string code = R"(
        let m = {"k": 7};
        m["k"] *= 3;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "21\n");
}

TEST(HashMap, CompoundIndexDivide) {
    std::string code = R"(
        let m = {"k": 18};
        m["k"] /= 6;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(HashMap, CompoundIndexXor) {
    std::string code = R"(
        let m = {"k": 5};
        m["k"] ^= 3;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(HashMap, CompoundIndexShiftLeft) {
    std::string code = R"(
        let m = {"k": 2};
        m["k"] <<= 3;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(HashMap, CompoundIndexShiftRight) {
    std::string code = R"(
        let m = {"k": 16};
        m["k"] >>= 2;
        print m["k"];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

// ==================== ERROR CASES ====================

TEST(HashMap, InvalidIndexType) {
    std::string code = R"(
        let map = {"key": "value"};
        print map[{}];  // Trying to use a hash map as an index
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "RUNTIME_ERROR: Hash map index must be a string, number, boolean, or nil");
}

TEST(HashMap, InvalidMemberAccess) {
    std::string code = R"(
        let arr = [1, 2, 3];
        print arr.keys();  // Arrays don't have keys method
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "RUNTIME_ERROR: Unknown array member: keys");
}

TEST(HashMap, NonObjectMemberAccess) {
    std::string code = R"(
        let num = 42;
        print num.size;  // Numbers don't have members
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "RUNTIME_ERROR: Only arrays, hash maps, and class instances have members");
}
