#include "interpreter.h"
#include "callable.h"
#include "stmt.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include "features/array.h"  // Added!
#include "features/hashmap.h"  // Added!
#include <memory>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <iostream>  // Added for std::cout, std::cin
#include <chrono>    // Added for clock() function
#include <sstream>   // Added for JSON encoding
#include <iomanip>   // Added for JSON formatting
#include <thread>    // Added for sleep function
#include <fstream>   // Added for file operations
#include <cstdio>    // Added for std::remove function

namespace volt {

Interpreter::Interpreter()
    : environment_(std::make_shared<Environment>()),
      globals_(environment_) {
    // Set up all the built-in functions that come with VoltScript
    defineNatives();
}

void Interpreter::reset() {
    environment_ = std::make_shared<Environment>();
    globals_ = environment_;
    defineNatives();
}

// Register native functions (built into the language)
void Interpreter::defineNatives() {
    // clock() - returns current time in seconds
    globals_->define("clock", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return static_cast<double>(millis) / 1000.0;
        },
        "clock"
    ));
    
    // len() - returns length of string, array, or hash map  // Added feature!
    globals_->define("len", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isString(args[0])) {
                return static_cast<double>(asString(args[0]).length());
            }
            if (isArray(args[0])) {  // Added!
                return static_cast<double>(asArray(args[0])->length());
            }
            if (isHashMap(args[0])) {  // Added!
                return static_cast<double>(asHashMap(args[0])->size());
            }
            throw std::runtime_error("len() requires a string, array, or hash map argument");
        },
        "len"
    ));
    
    // str(value) - convert to string
    globals_->define("str", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return valueToString(args[0]);
        },
        "str"
    ));
    
    // num(value) - convert to number
    globals_->define("num", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isNumber(args[0])) return args[0];
            if (isString(args[0])) {
                try {
                    return std::stod(asString(args[0]));
                } catch (...) {
                    throw std::runtime_error("Cannot convert string to number: " + asString(args[0]));
                }
            }
            if (isBool(args[0])) {
                return asBool(args[0]) ? 1.0 : 0.0;
            }
            throw std::runtime_error("Cannot convert to number");
        },
        "num"
    ));
    
    // input(prompt) - read line from stdin
    globals_->define("input", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isString(args[0])) {
                std::cout << asString(args[0]);
            }
            std::string line;
            std::getline(std::cin, line);
            return line;
        },
        "input"
    ));
    
    // readFile(path) - read entire file as string
    globals_->define("readFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("readFile() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            if (!file) {
                throw std::runtime_error("Could not open file: " + asString(args[0]));
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        },
        "readFile"
    ));
    
    // writeFile(path, content) - write string to file (overwrites)
    globals_->define("writeFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("writeFile() requires string path and content");
            }
            std::ofstream file(asString(args[0]));
            if (!file) {
                throw std::runtime_error("Could not open file for writing: " + asString(args[0]));
            }
            file << asString(args[1]);
            return true;
        },
        "writeFile"
    ));
    
    // appendFile(path, content) - append string to file
    globals_->define("appendFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("appendFile() requires string path and content");
            }
            std::ofstream file(asString(args[0]), std::ios::app);
            if (!file) {
                throw std::runtime_error("Could not open file for appending: " + asString(args[0]));
            }
            file << asString(args[1]);
            return true;
        },
        "appendFile"
    ));
    
    // fileExists(path) - check if file exists
    globals_->define("fileExists", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("fileExists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return file.good();
        },
        "fileExists"
    ));
    
    // toUpper(str) - convert string to uppercase
    globals_->define("toUpper", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("toUpper() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return s;
        },
        "toUpper"
    ));
    
    // toLower(str) - convert string to lowercase
    globals_->define("toLower", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("toLower() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        },
        "toLower"
    ));
    
    // upper(str) - convert string to uppercase (alias for toUpper)
    globals_->define("upper", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("upper() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return s;
        },
        "upper"
    ));
    
    // lower(str) - convert string to lowercase (alias for toLower)
    globals_->define("lower", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("lower() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        },
        "lower"
    ));
    
    // substr() - grab a piece of a string  // Added!
    globals_->define("substr", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("substr() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("substr() requires a number as start position");
            if (!isNumber(args[2])) throw std::runtime_error("substr() requires a number as length");
            
            std::string s = asString(args[0]);
            int start = static_cast<int>(asNumber(args[1]));
            int length = static_cast<int>(asNumber(args[2]));
            
            if (start < 0) start = 0;
            if (start >= static_cast<int>(s.length())) return std::string("");
            if (length < 0) length = 0;
            
            if (start + length > static_cast<int>(s.length())) {
                length = static_cast<int>(s.length()) - start;
            }
            
            return s.substr(start, length);
        },
        "substr"
    ));
    
    // indexOf() - find where a substring appears in a string  // Added!
    globals_->define("indexOf", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("indexOf() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("indexOf() requires a string as second argument");
            
            std::string s = asString(args[0]);
            std::string sub = asString(args[1]);
            
            size_t pos = s.find(sub);
            if (pos == std::string::npos) {
                return -1.0; // Return -1 if not found
            }
            
            return static_cast<double>(pos);
        },
        "indexOf"
    ));
    
    // ==================== BASIC MATH STUFF (NEW FOR v0.7.2) ====================
    
    // abs(number) - absolute value
    globals_->define("abs", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("abs() requires a number");
            return std::abs(asNumber(args[0]));
        },
        "abs"
    ));
    
    // sqrt(number) - square root
    globals_->define("sqrt", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("sqrt() requires a number");
            double val = asNumber(args[0]);
            if (val < 0) throw std::runtime_error("sqrt() argument must be non-negative");
            return std::sqrt(val);
        },
        "sqrt"
    ));
    
    // pow(base, exponent) - power function
    globals_->define("pow", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]) || !isNumber(args[1])) {
                throw std::runtime_error("pow() requires two numbers");
            }
            return std::pow(asNumber(args[0]), asNumber(args[1]));
        },
        "pow"
    ));
    
    // min(a, b) - minimum of two values
    globals_->define("min", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]) || !isNumber(args[1])) {
                throw std::runtime_error("min() requires two numbers");
            }
            return std::min(asNumber(args[0]), asNumber(args[1]));
        },
        "min"
    ));
    
    // max(a, b) - maximum of two values
    globals_->define("max", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]) || !isNumber(args[1])) {
                throw std::runtime_error("max() requires two numbers");
            }
            return std::max(asNumber(args[0]), asNumber(args[1]));
        },
        "max"
    ));
    
    // round(number) - round to nearest integer
    globals_->define("round", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("round() requires a number");
            return std::round(asNumber(args[0]));
        },
        "round"
    ));
    
    // floor(number) - round down to integer
    globals_->define("floor", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("floor() requires a number");
            return std::floor(asNumber(args[0]));
        },
        "floor"
    ));
    
    // ceil(number) - round up to integer
    globals_->define("ceil", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("ceil() requires a number");
            return std::ceil(asNumber(args[0]));
        },
        "ceil"
    ));
    
    // random() - random number between 0 and 1
    globals_->define("random", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            return static_cast<double>(std::rand()) / RAND_MAX;
        },
        "random"
    ));
    
    // ==================== TRIGONOMETRY STUFF (NEW FOR v0.7.5) ====================
    
    // sin(x) - sine function
    globals_->define("sin", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("sin() requires a number");
            return std::sin(asNumber(args[0]));
        },
        "sin"
    ));
    
    // cos(x) - cosine function
    globals_->define("cos", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("cos() requires a number");
            return std::cos(asNumber(args[0]));
        },
        "cos"
    ));
    
    // tan(x) - tangent function
    globals_->define("tan", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("tan() requires a number");
            return std::tan(asNumber(args[0]));
        },
        "tan"
    ));
    
    // ==================== LOG AND EXPONENTIAL STUFF (NEW FOR v0.7.5) ====================
    
    // log(x) - natural logarithm
    globals_->define("log", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("log() requires a number");
            double x = asNumber(args[0]);
            if (x <= 0) throw std::runtime_error("log() argument must be positive");
            return std::log(x);
        },
        "log"
    ));
    
    // exp(x) - exponential function
    globals_->define("exp", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("exp() requires a number");
            return std::exp(asNumber(args[0]));
        },
        "exp"
    ));
    
    // ==================== TIME AND DATE STUFF (NEW FOR v0.7.5) ====================
    
    // now() - get current timestamp in milliseconds
    globals_->define("now", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return static_cast<double>(millis);
        },
        "now"
    ));
    
    // formatDate(timestamp, format) - format timestamp (stub implementation)
    globals_->define("formatDate", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("formatDate() requires a timestamp number as first argument");
            if (!isString(args[1])) throw std::runtime_error("formatDate() requires a format string as second argument");
            // Simple implementation - just return a formatted string
            double timestamp = asNumber(args[0]);
            std::string format = asString(args[1]);
            std::ostringstream oss;
            oss << "Date(" << static_cast<long long>(timestamp) << ") formatted as '" << format << "'";
            return oss.str();
        },
        "formatDate"
    ));
    
    // ==================== JSON HANDLING (NEW FOR v0.7.5) ====================
    
    // jsonEncode(value) - encode value to JSON string
    globals_->define("jsonEncode", std::make_shared<NativeFunction>(
        1,
        [this](const std::vector<Value>& args) -> Value {
            return this->encodeToJson(args[0]);
        },
        "jsonEncode"
    ));
    
    // jsonDecode(jsonString) - decode JSON string to value
    globals_->define("jsonDecode", std::make_shared<NativeFunction>(
        1,
        [this](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("jsonDecode() requires a string");
            return this->decodeFromJson(asString(args[0]));
        },
        "jsonDecode"
    ));
    
    // ==================== STRING MANIPULATION (NEW FOR v0.7.2) ====================
    
    // trim(str) - remove whitespace from both ends
    globals_->define("trim", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("trim() requires a string");
            std::string s = asString(args[0]);
            
            // Remove leading whitespace
            size_t start = s.find_first_not_of(" \t\n\r\f\v");
            if (start == std::string::npos) return std::string(""); // All whitespace
            
            // Remove trailing whitespace
            size_t end = s.find_last_not_of(" \t\n\r\f\v");
            
            return s.substr(start, end - start + 1);
        },
        "trim"
    ));
    
    // split(str, delimiter) - split string into array
    globals_->define("split", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("split() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("split() requires a string delimiter");
            
            std::string s = asString(args[0]);
            std::string delim = asString(args[1]);
            
            auto resultArray = std::make_shared<VoltArray>();
            
            if (delim.empty()) {
                // Split into individual characters
                for (char c : s) {
                    resultArray->push(std::string(1, c));
                }
            } else {
                size_t start = 0;
                size_t pos = s.find(delim);
                
                while (pos != std::string::npos) {
                    resultArray->push(s.substr(start, pos - start));
                    start = pos + delim.length();
                    pos = s.find(delim, start);
                }
                resultArray->push(s.substr(start)); // Add remaining part
            }
            
            return resultArray;
        },
        "split"
    ));
    
    // replace(str, search, replacement) - replace all occurrences
    globals_->define("replace", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("replace() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("replace() requires a string to search for");
            if (!isString(args[2])) throw std::runtime_error("replace() requires a string replacement");
            
            std::string s = asString(args[0]);
            std::string search = asString(args[1]);
            std::string replacement = asString(args[2]);
            
            if (search.empty()) return s; // Nothing to replace
            
            std::string result = s;
            size_t pos = 0;
            
            while ((pos = result.find(search, pos)) != std::string::npos) {
                result.replace(pos, search.length(), replacement);
                pos += replacement.length();
                
                // Avoid infinite loop with empty replacement
                if (replacement.empty()) pos++;
            }
            
            return result;
        },
        "replace"
    ));
    
    // startsWith(str, prefix) - check if string starts with prefix
    globals_->define("startsWith", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("startsWith() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("startsWith() requires a string prefix");
            
            std::string s = asString(args[0]);
            std::string prefix = asString(args[1]);
            
            return s.length() >= prefix.length() && s.substr(0, prefix.length()) == prefix;
        },
        "startsWith"
    ));
    
    // endsWith(str, suffix) - check if string ends with suffix
    globals_->define("endsWith", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("endsWith() requires a string as first argument");
            if (!isString(args[1])) throw std::runtime_error("endsWith() requires a string suffix");
            
            std::string s = asString(args[0]);
            std::string suffix = asString(args[1]);
            
            return s.length() >= suffix.length() && 
                   s.substr(s.length() - suffix.length()) == suffix;
        },
        "endsWith"
    ));
    
    // type(val) - get type of value as string
    globals_->define("type", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            const Value& v = args[0];
            if (isNil(v)) return "nil";
            if (isBool(v)) return "bool";
            if (isNumber(v)) return "number";
            if (isString(v)) return "string";
            if (isCallable(v)) return "function";
            if (isArray(v)) return "array";
            if (isHashMap(v)) return "hashmap";  // Added!
            return "unknown";
        },
        "type"
    ));
    
    // keys(hashmap) - get all keys from a hash map  // Added!
    globals_->define("keys", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("keys() requires a hashmap argument");
            }
            
            auto map = asHashMap(args[0]);
            auto keysVec = map->getKeys();
            
            // Create an array with the keys
            auto resultArray = std::make_shared<VoltArray>();
            for (const auto& key : keysVec) {
                resultArray->push(key);
            }
            
            return resultArray;
        },
        "keys"
    ));
    
    // values(hashmap) - get all values from a hash map  // Added!
    globals_->define("values", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("values() requires a hashmap argument");
            }
            
            auto map = asHashMap(args[0]);
            auto valuesVec = map->getValues();
            
            // Create an array with the values
            auto resultArray = std::make_shared<VoltArray>();
            for (const auto& value : valuesVec) {
                resultArray->push(value);
            }
            
            return resultArray;
        },
        "values"
    ));
    
    // has(hashmap, key) - check if a key exists in a hash map  // Added!
    globals_->define("has", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("has() requires a hashmap as first argument");
            }
            if (!isString(args[1]) && !isNumber(args[1]) && !isBool(args[1]) && !isNil(args[1])) {
                throw std::runtime_error("has() requires a string, number, boolean, or nil as key");
            }
            
            auto map = asHashMap(args[0]);
            
            // Convert key to string
            std::string keyStr = valueToString(args[1]);
            
            return map->contains(keyStr);
        },
        "has"
    ));
    
    // remove(hashmap, key) - remove a key-value pair from a hash map  // Added!
    globals_->define("remove", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("remove() requires a hashmap as first argument");
            }
            if (!isString(args[1]) && !isNumber(args[1]) && !isBool(args[1]) && !isNil(args[1])) {
                throw std::runtime_error("remove() requires a string, number, boolean, or nil as key");
            }
            
            auto map = asHashMap(args[0]);
            
            // Convert key to string
            std::string keyStr = valueToString(args[1]);
            
            return map->remove(keyStr);  // Returns true if removed, false if not found
        },
        "remove"
    ));
    
    // ==================== FILE I/O ENHANCEMENTS (NEW FOR v0.7.9) ====================
    
    // exists(path) - check if file or directory exists
    globals_->define("exists", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("exists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return file.good();
        },
        "exists"
    ));
    
    // deleteFile(path) - delete a file
    globals_->define("deleteFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("deleteFile() requires a string path");
            }
            std::string path = asString(args[0]);
            if (std::remove(path.c_str()) == 0) {
                return true;
            } else {
                return false;
            }
        },
        "deleteFile"
    ));
    
    // fileSize(path) - get size of a file in bytes
    globals_->define("fileSize", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("fileSize() requires a string path");
            }
            std::string path = asString(args[0]);
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) {
                throw std::runtime_error("Could not open file: " + path);
            }
            return static_cast<double>(file.tellg());
        },
        "fileSize"
    ));
    
    // ==================== ADVANCED STRING FUNCTIONS (NEW FOR v0.7.9) ====================
    
    // padStart(str, targetLength, padString) - pad string from start
    globals_->define("padStart", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("padStart() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("padStart() requires a number as target length");
            if (!isString(args[2])) throw std::runtime_error("padStart() requires a string as padding");
            
            std::string str = asString(args[0]);
            int targetLen = static_cast<int>(asNumber(args[1]));
            std::string padStr = asString(args[2]);
            
            if (targetLen <= static_cast<int>(str.length()) || padStr.empty()) {
                return str;
            }
            
            int padLen = targetLen - str.length();
            std::string result = "";
            while (result.length() < padLen) {
                result += padStr;
            }
            result = result.substr(0, padLen);
            
            return result + str;
        },
        "padStart"
    ));
    
    // padEnd(str, targetLength, padString) - pad string from end
    globals_->define("padEnd", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("padEnd() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("padEnd() requires a number as target length");
            if (!isString(args[2])) throw std::runtime_error("padEnd() requires a string as padding");
            
            std::string str = asString(args[0]);
            int targetLen = static_cast<int>(asNumber(args[1]));
            std::string padStr = asString(args[2]);
            
            if (targetLen <= static_cast<int>(str.length()) || padStr.empty()) {
                return str;
            }
            
            int padLen = targetLen - str.length();
            std::string result = "";
            while (result.length() < padLen) {
                result += padStr;
            }
            result = result.substr(0, padLen);
            
            return str + result;
        },
        "padEnd"
    ));
    
    // repeat(str, count) - repeat string n times
    globals_->define("repeat", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("repeat() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("repeat() requires a number as count");
            
            std::string str = asString(args[0]);
            int count = static_cast<int>(asNumber(args[1]));
            
            if (count <= 0) return std::string("");
            
            std::string result = "";
            for (int i = 0; i < count; i++) {
                result += str;
            }
            
            return result;
        },
        "repeat"
    ));
    
    // charCodeAt(str, index) - get character code at index
    globals_->define("charCodeAt", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("charCodeAt() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("charCodeAt() requires a number as index");
            
            std::string str = asString(args[0]);
            int index = static_cast<int>(asNumber(args[1]));
            
            if (index < 0 || index >= static_cast<int>(str.length())) {
                return -1.0; // Return -1 if index is out of bounds
            }
            
            return static_cast<double>(static_cast<unsigned char>(str[index]));
        },
        "charCodeAt"
    ));
    
    // fromCharCode(code) - create string from character code
    globals_->define("fromCharCode", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("fromCharCode() requires a number");
            
            int code = static_cast<int>(asNumber(args[0]));
            
            if (code < 0 || code > 255) {
                throw std::runtime_error("Character code must be between 0 and 255");
            }
            
            return std::string(1, static_cast<char>(code));
        },
        "fromCharCode"
    ));
    
    // ==================== FUNCTIONAL PROGRAMMING UTILITIES (NEW FOR v0.7.9) ====================
    
    // compose(...functions) - compose functions from right to left
    globals_->define("compose", std::make_shared<NativeFunction>(
        -1, // Variable arity
        [this](const std::vector<Value>& args) -> Value {
            // Verify all arguments are callable
            for (const auto& arg : args) {
                if (!isCallable(arg)) {
                    throw std::runtime_error("All arguments to compose() must be functions");
                }
            }
            
            // Create a function that applies the composed functions
            return std::make_shared<NativeFunction>(
                1, // Takes one argument
                [this, args](const std::vector<Value>& callArgs) -> Value {
                    if (callArgs.empty()) {
                        throw std::runtime_error("compose() function needs at least one argument");
                    }
                    
                    Value result = callArgs[0];
                    
                    // Apply functions from right to left (last to first)
                    for (int i = args.size() - 1; i >= 0; i--) {
                        auto func = asCallable(args[i]);
                        result = func->call(*this, {result});
                    }
                    
                    return result;
                },
                "composedFunction"
            );
        },
        "compose"
    ));
    
    // pipe(...functions) - pipe value through functions from left to right
    globals_->define("pipe", std::make_shared<NativeFunction>(
        -1, // Variable arity
        [this](const std::vector<Value>& args) -> Value {
            // Verify all arguments are callable
            for (const auto& arg : args) {
                if (!isCallable(arg)) {
                    throw std::runtime_error("All arguments to pipe() must be functions");
                }
            }
            
            // Create a function that pipes the value through functions
            return std::make_shared<NativeFunction>(
                1, // Takes one argument
                [this, args](const std::vector<Value>& callArgs) -> Value {
                    if (callArgs.empty()) {
                        throw std::runtime_error("pipe() function needs at least one argument");
                    }
                    
                    Value result = callArgs[0];
                    
                    // Apply functions from left to right (first to last)
                    for (size_t i = 0; i < args.size(); i++) {
                        auto func = asCallable(args[i]);
                        result = func->call(*this, {result});
                    }
                    
                    return result;
                },
                "pipeFunction"
            );
        },
        "pipe"
    ));
    
    // ==================== PERFORMANCE UTILITIES (NEW FOR v0.7.9) ====================
    
    // sleep(milliseconds) - pause execution for given milliseconds
    globals_->define("sleep", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) {
                throw std::runtime_error("sleep() requires a number (milliseconds)");
            }
            int ms = static_cast<int>(asNumber(args[0]));
            if (ms < 0) ms = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            return true;
        },
        "sleep"
    ));
    
    // benchmark(func, ...args) - measure execution time of function
    globals_->define("benchmark", std::make_shared<NativeFunction>(
        -1, // Variable arity: function + any number of arguments
        [this](const std::vector<Value>& args) -> Value {
            if (args.empty() || !isCallable(args[0])) {
                throw std::runtime_error("benchmark() requires a function as first argument");
            }
            
            auto func = asCallable(args[0]);
            std::vector<Value> callArgs(args.begin() + 1, args.end());
            
            auto start = std::chrono::high_resolution_clock::now();
            Value result = func->call(*this, callArgs);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            // Return an object with result and execution time
            auto resultMap = std::make_shared<VoltHashMap>();
            resultMap->set("result", result);
            resultMap->set("timeMicroseconds", static_cast<double>(duration.count()));
            resultMap->set("timeMilliseconds", static_cast<double>(duration.count()) / 1000.0);
            
            return resultMap;
        },
        "benchmark"
    ));
}

// ========================================
// STATEMENT EXECUTION
// ========================================

void Interpreter::execute(Stmt* stmt) {
    if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        executeExprStmt(exprStmt);
    } else if (auto* printStmt = dynamic_cast<PrintStmt*>(stmt)) {
        executePrintStmt(printStmt);
    } else if (auto* letStmt = dynamic_cast<LetStmt*>(stmt)) {
        executeLetStmt(letStmt);
    } else if (auto* blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        executeBlockStmt(blockStmt);
    } else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        executeIfStmt(ifStmt);
    } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        executeWhileStmt(whileStmt);
    } else if (auto* runUntilStmt = dynamic_cast<RunUntilStmt*>(stmt)) {
        executeRunUntilStmt(runUntilStmt);
    } else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt)) {
        executeForStmt(forStmt);
    } else if (auto* fnStmt = dynamic_cast<FnStmt*>(stmt)) {
        executeFnStmt(fnStmt);
    } else if (auto* returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        executeReturnStmt(returnStmt);
    } else if (auto* breakStmt = dynamic_cast<BreakStmt*>(stmt)) {
        executeBreakStmt(breakStmt);
    } else if (auto* continueStmt = dynamic_cast<ContinueStmt*>(stmt)) {
        executeContinueStmt(continueStmt);
    } else {
        throw std::runtime_error("Unknown statement type");
    }
}

void Interpreter::execute(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt.get());
    }
}

void Interpreter::executeExprStmt(ExprStmt* stmt) {
    evaluate(stmt->expr.get());
}

void Interpreter::executePrintStmt(PrintStmt* stmt) {
    Value value = evaluate(stmt->expr.get());
    std::cout << valueToString(value) << "\n";
}

void Interpreter::executeLetStmt(LetStmt* stmt) {
    Value value = nullptr;
    if (stmt->initializer) {
        value = evaluate(stmt->initializer.get());
    }
    environment_->define(stmt->name, value);
}

void Interpreter::executeBlockStmt(BlockStmt* stmt) {
    executeBlock(stmt->statements,
                 std::make_shared<Environment>(environment_));
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements,
                                std::shared_ptr<Environment> environment) {
    std::shared_ptr<Environment> previous = environment_;
    try {
        environment_ = environment;
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
        environment_ = previous;
    } catch (...) {
        environment_ = previous;
        throw;
    }
}

void Interpreter::executeIfStmt(IfStmt* stmt) {
    Value condition = evaluate(stmt->condition.get());
    if (isTruthy(condition)) {
        execute(stmt->thenBranch.get());
    } else if (stmt->elseBranch) {
        execute(stmt->elseBranch.get());
    }
}

void Interpreter::executeWhileStmt(WhileStmt* stmt) {
    while (isTruthy(evaluate(stmt->condition.get()))) {
        try {
            execute(stmt->body.get());
        } catch (const ContinueException&) {
            continue; // Continue to next iteration
        } catch (const BreakException&) {
            break; // Exit the loop
        }
    }
}

void Interpreter::executeRunUntilStmt(RunUntilStmt* stmt) {
    // Run-until: executes body at least once, then continues until condition becomes TRUE
    // This is different from do-while which continues while condition is true
    do {
        try {
            execute(stmt->body.get());
        } catch (const ContinueException&) {
            continue; // Continue to next iteration
        } catch (const BreakException&) {
            break; // Exit the loop
        }
    } while (!isTruthy(evaluate(stmt->condition.get())));
}

void Interpreter::executeForStmt(ForStmt* stmt) {
    // Create new scope for loop
    auto loopEnv = std::make_shared<Environment>(environment_);
    auto previous = environment_;
    try {
        environment_ = loopEnv;
        
        // Execute initializer
        if (stmt->initializer) {
            execute(stmt->initializer.get());
        }
        
        // Condition (default to true if omitted)
        auto checkCondition = [&]() {
            if (stmt->condition) {
                return isTruthy(evaluate(stmt->condition.get()));
            }
            return true;
        };
        
        // Loop with break/continue support
        while (checkCondition()) {
            try {
                execute(stmt->body.get());
            } catch (const ContinueException&) {
                // Continue - execute increment and check condition
            } catch (const BreakException&) {
                break; // Exit the loop
            }
            
            // Execute increment
            if (stmt->increment) {
                evaluate(stmt->increment.get());
            }
        }
        
        environment_ = previous;
    } catch (...) {
        environment_ = previous;
        throw;
    }
}

void Interpreter::executeFnStmt(FnStmt* stmt) {
    // Create a function object that captures the current environment
    // This is what makes closures work!
    auto function = std::make_shared<VoltFunction>(stmt, environment_);
    
    // Define the function in the current scope
    // Note: We define it AFTER creating the closure, but that's okay
    // because the function name isn't in scope inside its own body
    // (unless you reference it for recursion, which we handle specially)
    environment_->define(stmt->name, function);
}

void Interpreter::executeReturnStmt(ReturnStmt* stmt) {
    Value value = nullptr;
    if (stmt->value) {
        value = evaluate(stmt->value.get());
    }
    
    // Throw a special exception to unwind the call stack
    // This is caught in VoltFunction::call()
    throw ReturnValue(value);
}
// ========================================
// EXPRESSION EVALUATION
// ========================================

Value Interpreter::evaluate(Expr* expr) {
    if (auto* lit = dynamic_cast<LiteralExpr*>(expr)) {
        return evaluateLiteral(lit);
    }
    
    if (auto* var = dynamic_cast<VariableExpr*>(expr)) {
        return evaluateVariable(var);
    }
    
    if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
        return evaluateUnary(unary);
    }
    
    if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    }
    
    if (auto* logical = dynamic_cast<LogicalExpr*>(expr)) {
        return evaluateLogical(logical);
    }
    
    if (auto* group = dynamic_cast<GroupingExpr*>(expr)) {
        return evaluateGrouping(group);
    }
    
    if (auto* call = dynamic_cast<CallExpr*>(expr)) {
        return evaluateCall(call);
    }
    
    if (auto* assign = dynamic_cast<AssignExpr*>(expr)) {
        return evaluateAssign(assign);
    }
    
    if (auto* compound = dynamic_cast<CompoundAssignExpr*>(expr)) {
        return evaluateCompoundAssign(compound);
    }
    
    if (auto* update = dynamic_cast<UpdateExpr*>(expr)) {
        return evaluateUpdate(update);
    }
    
    if (auto* ternary = dynamic_cast<TernaryExpr*>(expr)) {
        return evaluateTernary(ternary);
    }
    
    // ========================================
    // ARRAY EXPRESSIONS - Added!
    // ========================================
    
    if (auto* array = dynamic_cast<ArrayExpr*>(expr)) {
        return evaluateArray(array);
    }
    
    if (auto* index = dynamic_cast<IndexExpr*>(expr)) {
        return evaluateIndex(index);
    }
    
    if (auto* indexAssign = dynamic_cast<IndexAssignExpr*>(expr)) {
        return evaluateIndexAssign(indexAssign);
    }
    
    if (auto* member = dynamic_cast<MemberExpr*>(expr)) {
        return evaluateMember(member);
    }
    
    // ========================================
    // HASH MAP EXPRESSIONS - Added!
    // ========================================
    
    if (auto* hashMap = dynamic_cast<HashMapExpr*>(expr)) {
        return evaluateHashMap(hashMap);
    }
    
    throw std::runtime_error("Unknown expression type");
}

Value Interpreter::evaluateLiteral(LiteralExpr* expr) {
    switch (expr->type) {
        case LiteralExpr::Type::Number:
            return expr->numberValue;
        case LiteralExpr::Type::String:
            return expr->stringValue;
        case LiteralExpr::Type::Bool:
            return expr->boolValue;
        case LiteralExpr::Type::Nil:
            return nullptr;
    }
    return nullptr;
}

Value Interpreter::evaluateVariable(VariableExpr* expr) {
    try {
        return environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
}

Value Interpreter::evaluateUnary(UnaryExpr* expr) {
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        case TokenType::Minus:
            checkNumberOperand(expr->op, right);
            return -asNumber(right);
        case TokenType::Bang:
            return !isTruthy(right);
        default:
            throw RuntimeError(expr->op, "Unknown unary operator");
    }
}

Value Interpreter::evaluateBinary(BinaryExpr* expr) {
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        case TokenType::Plus:
            if (isNumber(left) && isNumber(right)) {
                return asNumber(left) + asNumber(right);
            }
            if (isString(left) && isString(right)) {
                return asString(left) + asString(right);
            }
            // Type coercion: string + number or number + string
            if (isString(left) && isNumber(right)) {
                return asString(left) + valueToString(right);
            }
            if (isNumber(left) && isString(right)) {
                return valueToString(left) + asString(right);
            }
            throw RuntimeError(expr->op, "Operands must be two numbers or two strings");
            
        case TokenType::Minus:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) - asNumber(right);
        case TokenType::Star:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) * asNumber(right);
        case TokenType::Slash:
            checkNumberOperands(expr->op, left, right);
            if (asNumber(right) == 0.0) {
                throw RuntimeError(expr->op, "Division by zero");
            }
            return asNumber(left) / asNumber(right);
        case TokenType::Percent:
            checkNumberOperands(expr->op, left, right);
            return std::fmod(asNumber(left), asNumber(right));
            
        case TokenType::Greater:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) > asNumber(right);
        case TokenType::GreaterEqual:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) >= asNumber(right);
        case TokenType::Less:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) < asNumber(right);
        case TokenType::LessEqual:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) <= asNumber(right);
            
        case TokenType::EqualEqual:
            return isEqual(left, right);
        case TokenType::BangEqual:
            return !isEqual(left, right);
            
        default:
            throw RuntimeError(expr->op, "Unknown binary operator");
    }
}

Value Interpreter::evaluateLogical(LogicalExpr* expr) {
    Value left = evaluate(expr->left.get());
    
    // Short-circuit evaluation
    if (expr->op.type == TokenType::Or) {
        if (isTruthy(left)) return left;
    } else {
        if (!isTruthy(left)) return left;
    }
    
    return evaluate(expr->right.get());
}

Value Interpreter::evaluateGrouping(GroupingExpr* expr) {
    return evaluate(expr->expr.get());
}

Value Interpreter::evaluateCall(CallExpr* expr) {
    // Evaluate the callee (the thing being called)
    Value callee = evaluate(expr->callee.get());
    
    // Evaluate all the arguments
    std::vector<Value> arguments;
    for (const auto& arg : expr->arguments) {
        arguments.push_back(evaluate(arg.get()));
    }
    
    // Make sure it's actually a function
    if (!isCallable(callee)) {
        throw RuntimeError(
            expr->token,
            "Can only call functions and classes"
        );
    }
    
    auto function = std::get<std::shared_ptr<Callable>>(callee);
    
    // Check arity (number of arguments)
    if (static_cast<int>(arguments.size()) != function->arity()) {
        throw RuntimeError(
            expr->token,
            "Expected " + std::to_string(function->arity()) +
            " arguments but got " + std::to_string(arguments.size())
        );
    }
    
    // Call the function!
    return function->call(*this, arguments);
}

Value Interpreter::evaluateAssign(AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    try {
        environment_->assign(expr->name, value);
    } catch (const std::runtime_error&) {
        // If variable doesn't exist, create it (implicit declaration)
        environment_->define(expr->name, value);
    }
    return value;
}

Value Interpreter::evaluateCompoundAssign(CompoundAssignExpr* expr) {
    Value current;
    try {
        current = environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    
    Value operand = evaluate(expr->value.get());
    Value result;
    
    switch (expr->op.type) {
        case TokenType::PlusEqual:
            if (isNumber(current) && isNumber(operand)) {
                result = asNumber(current) + asNumber(operand);
            } else if (isString(current) && isString(operand)) {
                result = asString(current) + asString(operand);
            } else if (isString(current) && isNumber(operand)) {
                result = asString(current) + valueToString(operand);
            } else {
                throw RuntimeError(expr->op, "Operands must be compatible for +=");
            }
            break;
        case TokenType::MinusEqual:
            checkNumberOperands(expr->op, current, operand);
            result = asNumber(current) - asNumber(operand);
            break;
        case TokenType::StarEqual:
            checkNumberOperands(expr->op, current, operand);
            result = asNumber(current) * asNumber(operand);
            break;
        case TokenType::SlashEqual:
            checkNumberOperands(expr->op, current, operand);
            if (asNumber(operand) == 0.0) {
                throw RuntimeError(expr->op, "Division by zero");
            }
            result = asNumber(current) / asNumber(operand);
            break;
        default:
            throw RuntimeError(expr->op, "Unknown compound assignment operator");
    }
    
    try {
        environment_->assign(expr->name, result);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    return result;
}

Value Interpreter::evaluateUpdate(UpdateExpr* expr) {
    Value current;
    try {
        current = environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    
    if (!isNumber(current)) {
        throw RuntimeError(expr->op, "Operand must be a number for increment/decrement");
    }
    
    double oldValue = asNumber(current);
    double newValue;
    if (expr->op.type == TokenType::PlusPlus) {
        newValue = oldValue + 1;
    } else {
        newValue = oldValue - 1;
    }
    
    try {
        environment_->assign(expr->name, newValue);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    
    // Return old value for postfix, new value for prefix
    return expr->prefix ? newValue : oldValue;
}

Value Interpreter::evaluateTernary(TernaryExpr* expr) {
    if (isTruthy(evaluate(expr->condition.get()))) {
        return evaluate(expr->thenBranch.get());
    }
    return evaluate(expr->elseBranch.get());
}

// ========================================
// ARRAY EVALUATION - NEW METHODS!
// ========================================

Value Interpreter::evaluateArray(ArrayExpr* expr) {
    std::vector<Value> elements;
    
    // Evaluate all element expressions
    for (const auto& elem : expr->elements) {
        elements.push_back(evaluate(elem.get()));
    }
    
    // Create and return array
    return std::make_shared<VoltArray>(elements);
}

Value Interpreter::evaluateIndex(IndexExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Index must be a number
        if (!isNumber(index)) {
            throw RuntimeError(expr->token, "Array index must be a number");
        }
        
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= array->length()) {
            throw RuntimeError(expr->token, "Array index out of bounds: " + std::to_string(idx));
        }
        
        return array->get(idx);
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Convert index to string key
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            // Convert number to string representation
            double num = asNumber(index);
            if (num == static_cast<long long>(num)) {
                key = std::to_string(static_cast<long long>(num));
            } else {
                key = std::to_string(num);
                // Remove trailing zeros after decimal point
                key.erase(key.find_last_not_of('0') + 1, std::string::npos);
                key.erase(key.find_last_not_of('.') + 1, std::string::npos);
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throw RuntimeError(expr->token, "Hash map index must be a string, number, boolean, or nil");
        }
        
        return map->get(key);
    }
    
    throw RuntimeError(expr->token, "Can only index arrays and hash maps");
}

Value Interpreter::evaluateIndexAssign(IndexAssignExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    Value value = evaluate(expr->value.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Index must be a number
        if (!isNumber(index)) {
            throw RuntimeError(expr->token, "Array index must be a number");
        }
        
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= array->length()) {
            throw RuntimeError(expr->token, "Array index out of bounds: " + std::to_string(idx));
        }
        
        array->set(idx, value);
        return value;
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Convert index to string key
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            // Convert number to string representation
            if (asNumber(index) == static_cast<long long>(asNumber(index))) {
                key = std::to_string(static_cast<long long>(asNumber(index)));
            } else {
                key = std::to_string(asNumber(index));
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throw RuntimeError(expr->token, "Hash map index must be a string, number, boolean, or nil");
        }
        
        map->set(key, value);
        return value;
    }
    
    throw RuntimeError(expr->token, "Can only index arrays and hash maps");
}

Value Interpreter::evaluateMember(MemberExpr* expr) {
    Value object = evaluate(expr->object.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Handle array.length
        if (expr->member == "length") {
            return static_cast<double>(array->length());
        }
        
        // Handle array.push - return a callable
        if (expr->member == "push") {
            return std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    array->push(args[0]);
                    return nullptr; // returns nil
                },
                "push"
            );
        }
        
        // Handle array.pop
        if (expr->member == "pop") {
            return std::make_shared<NativeFunction>(
                0,
                [array](const std::vector<Value>&) -> Value {
                    return array->pop();
                },
                "pop"
            );
        }
        
        // Handle array.reverse
        if (expr->member == "reverse") {
            return std::make_shared<NativeFunction>(
                0,
                [array](const std::vector<Value>&) -> Value {
                    array->reverse();
                    return nullptr;
                },
                "reverse"
            );
        }
        
        throw RuntimeError(expr->token, "Unknown array member: " + expr->member);
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Handle hash map properties/methods
        if (expr->member == "size") {
            return static_cast<double>(map->size());
        }
        
        if (expr->member == "keys") {
            return std::make_shared<NativeFunction>(
                0,
                [map](const std::vector<Value>&) -> Value {
                    auto keysVec = map->getKeys();
                    
                    // Create an array with the keys
                    auto resultArray = std::make_shared<VoltArray>();
                    for (const auto& key : keysVec) {
                        resultArray->push(key);
                    }
                    
                    return resultArray;
                },
                "hashmap.keys"
            );
        }
        
        if (expr->member == "values") {
            return std::make_shared<NativeFunction>(
                0,
                [map](const std::vector<Value>&) -> Value {
                    auto valuesVec = map->getValues();
                    
                    // Create an array with the values
                    auto resultArray = std::make_shared<VoltArray>();
                    for (const auto& value : valuesVec) {
                        resultArray->push(value);
                    }
                    
                    return resultArray;
                },
                "hashmap.values"
            );
        }
        
        if (expr->member == "has") {  // Added!
            return std::make_shared<NativeFunction>(
                1,
                [map](const std::vector<Value>& args) -> Value {
                    // Convert key to string
                    std::string keyStr = valueToString(args[0]);
                    return map->contains(keyStr);
                },
                "hashmap.has"
            );
        }
        
        if (expr->member == "remove") {  // Added!
            return std::make_shared<NativeFunction>(
                1,
                [map](const std::vector<Value>& args) -> Value {
                    // Convert key to string
                    std::string keyStr = valueToString(args[0]);
                    return map->remove(keyStr);  // Returns true if removed, false if not found
                },
                "hashmap.remove"
            );
        }
        
        throw RuntimeError(expr->token, "Unknown hash map member: " + expr->member);
    }
    
    throw RuntimeError(expr->token, "Only arrays and hash maps have members");
}

// Evaluate hash map literal expression  // Added!
Value Interpreter::evaluateHashMap(HashMapExpr* expr) {
    auto hashMap = std::make_shared<VoltHashMap>();
    
    for (const auto& [keyExpr, valueExpr] : expr->keyValuePairs) {
        Value key = evaluate(keyExpr.get());
        Value value = evaluate(valueExpr.get());
        
        // Convert key to string representation (for storage in hash map)
        std::string keyStr = valueToString(key);  // Use the same string representation as valueToString
        
        hashMap->set(keyStr, value);
    }
    
    return hashMap;
}

void Interpreter::executeBreakStmt(BreakStmt*) {
    throw BreakException();
}

void Interpreter::executeContinueStmt(ContinueStmt*) {
    throw ContinueException();
}

void Interpreter::checkNumberOperand(const Token& op, const Value& operand) {
    if (isNumber(operand)) return;
    throw RuntimeError(op, "Operand must be a number");
}

void Interpreter::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (isNumber(left) && isNumber(right)) return;
    throw RuntimeError(op, "Operands must be numbers");
}

// ==================== JSON ENCODING/DECODING METHODS ====================

std::string Interpreter::encodeToJson(const Value& value) {
    std::ostringstream oss;
    encodeJsonValue(value, oss);
    return oss.str();
}

Value Interpreter::decodeFromJson(const std::string& jsonString) {
    // Simple JSON decoder - handles basic types
    std::string trimmed = jsonString;
    // Remove leading/trailing whitespace
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    
    if (trimmed.empty()) {
        return nullptr;
    }
    
    // Handle null
    if (trimmed == "null") {
        return nullptr;
    }
    
    // Handle boolean
    if (trimmed == "true") {
        return true;
    }
    if (trimmed == "false") {
        return false;
    }
    
    // Handle string (quoted)
    if (trimmed.front() == '"' && trimmed.back() == '"') {
        // Remove quotes and handle escape sequences
        std::string str = trimmed.substr(1, trimmed.length() - 2);
        // Simple escape sequence handling
        std::string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[++i]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += str[i-1]; result += str[i]; break;
                }
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    // Handle number
    try {
        // Check if it's a valid number
        size_t pos;
        double num = std::stod(trimmed, &pos);
        if (pos == trimmed.length()) {
            return num;
        }
    } catch (...) {
        // Not a valid number
    }
    
    // Handle array
    if (trimmed.front() == '[' && trimmed.back() == ']') {
        // Parse array elements
        std::string content = trimmed.substr(1, trimmed.length() - 2);
        auto array = std::make_shared<VoltArray>();
        
        // Simple CSV-like parsing (doesn't handle nested structures well)
        std::istringstream iss(content);
        std::string element;
        while (std::getline(iss, element, ',')) {
            // Trim whitespace
            element.erase(0, element.find_first_not_of(" \t\n\r"));
            element.erase(element.find_last_not_of(" \t\n\r") + 1);
            if (!element.empty()) {
                array->push(decodeFromJson(element));
            }
        }
        return array;
    }
    
    // Handle object
    if (trimmed.front() == '{' && trimmed.back() == '}') {
        // Parse object key-value pairs
        // This is a simplified implementation
        return std::make_shared<VoltHashMap>(); // Return empty hashmap for now
    }
    
    // If we can't parse it, treat as string
    return trimmed;
}

void Interpreter::encodeJsonValue(const Value& value, std::ostringstream& oss) {
    if (isNil(value)) {
        oss << "null";
    } else if (isBool(value)) {
        oss << (asBool(value) ? "true" : "false");
    } else if (isNumber(value)) {
        double num = asNumber(value);
        if (num == static_cast<long long>(num)) {
            oss << static_cast<long long>(num);
        } else {
            oss << std::fixed << std::setprecision(6) << num;
            // Remove trailing zeros
            std::string str = oss.str();
            oss.str("");
            oss.clear();
            size_t dotPos = str.find('.');
            if (dotPos != std::string::npos) {
                size_t lastNonZero = str.find_last_not_of('0');
                if (lastNonZero == dotPos) {
                    str = str.substr(0, dotPos);
                } else if (lastNonZero != std::string::npos) {
                    str = str.substr(0, lastNonZero + 1);
                }
            }
            oss << str;
        }
    } else if (isString(value)) {
        std::string str = asString(value);
        oss << "\"";
        for (char c : str) {
            switch (c) {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '/': oss << "\/"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (c >= 0 && c < 32) {
                        // Escape control characters
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        oss << c;
                    }
                    break;
            }
        }
        oss << "\"";
    } else if (isArray(value)) {
        auto array = asArray(value);
        oss << "[";
        const auto& elements = array->elements();
        for (size_t i = 0; i < elements.size(); i++) {
            if (i > 0) oss << ",";
            encodeJsonValue(elements[i], oss);
        }
        oss << "]";
    } else if (isHashMap(value)) {
        oss << "{}"; // Simplified - just return empty object
    } else {
        oss << "\"" << valueToString(value) << "\"";
    }
}

} // namespace volt
