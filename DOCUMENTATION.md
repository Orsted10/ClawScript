# VoltScript Language Documentation

<div align="center">

## ⚡ VoltScript v0.7.9 ⚡
### *A Modern Programming Language Built from Scratch*

[![Tests](https://img.shields.io/badge/tests-450%2B-brightgreen)]()
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

</div>

---

## 📚 Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Language Basics](#language-basics)
4. [Data Types](#data-types)
5. [Control Flow](#control-flow)
6. [Functions](#functions)
7. [Arrays](#arrays)
8. [Hash Maps](#hash-maps)
9. [File I/O](#file-io)
10. [String Operations](#string-operations)
11. [Mathematical Functions](#mathematical-functions)
12. [Functional Programming](#functional-programming)
13. [Performance Tools](#performance-tools)
14. [Error Handling](#error-handling)
15. [Built-in Functions](#built-in-functions)
16. [Best Practices](#best-practices)
17. [Examples](#examples)
18. [FAQ](#faq)

---

## 🧠 Introduction

### What is VoltScript?

VoltScript is a dynamically-typed programming language designed to be both powerful and educational. Built entirely from scratch in C++20, it demonstrates how programming languages work under the hood while providing practical features for real-world programming.

### Key Features

- **Dynamic Typing**: Variables can hold any type of value
- **First-Class Functions**: Functions are values that can be assigned and passed around
- **Closures**: Functions that capture their surrounding environment
- **Dynamic Arrays**: Resizable collections with built-in methods
- **Hash Maps**: Key-value data structures
- **Lexical Scoping**: Variables are scoped to their blocks
- **Precise Error Reporting**: Errors point to exact source locations

### Design Philosophy

VoltScript follows these principles:

1. **Clarity over cleverness** - Code should be readable and understandable
2. **Explicit behavior** - No hidden magic or implicit conversions
3. **Educational value** - Every feature demonstrates a fundamental concept
4. **Practical utility** - Real-world programming capabilities

---

## 🚀 Getting Started

### Installation

#### Requirements

- C++ compiler with C++20 support
- CMake 3.14 or higher
- Git (for cloning the repository)

#### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/voltscript.git
cd voltscript

# Create build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build

# Run the interpreter
./build/bin/volt
```

### Running VoltScript

#### Interactive Mode (REPL)

```bash
./build/bin/volt
```

This starts an interactive session where you can type and execute VoltScript code directly:

```
⚡ VoltScript v0.7.9 REPL
Type 'exit' to quit
>> let x = 42;
>> print x;
42
>> 
```

#### Running Script Files

```bash
./build/bin/volt script.volt
```

Create a file called `hello.volt`:

```volt
print "Hello, VoltScript!";
```

Then run it:

```bash
./build/bin/volt hello.volt
# Output: Hello, VoltScript!
```

---

## 📖 Language Basics

### Variables and Assignment

Variables are declared using the `let` keyword:

```volt
let name = "Alice";
let age = 25;
let isActive = true;
let score = 95.5;
let nothing = nil;
```

Variables can be reassigned:

```volt
let x = 10;
x = 20;  // Valid - reassignment
print x; // 20
```

### Comments

Single-line comments start with `//`:

```volt
// This is a comment
let x = 42; // This is also a comment
```

### Basic Operations

```volt
// Arithmetic
let sum = 10 + 5;        // 15
let diff = 10 - 3;       // 7
let product = 4 * 6;     // 24
let quotient = 15 / 3;   // 5
let remainder = 17 % 5;  // 2

// Comparison
let isEqual = (5 == 5);     // true
let isNotEqual = (5 != 3);  // true
let isGreater = (10 > 5);   // true
let isLess = (3 < 7);       // true

// Logical operations
let andResult = (true && false);  // false
let orResult = (true || false);   // true
let notResult = !true;            // false
```

---

## 📊 Data Types

### Nil

Represents the absence of a value:

```volt
let empty = nil;
print empty;  // nil
```

### Boolean

Logical true/false values:

```volt
let isTrue = true;
let isFalse = false;
print isTrue;   // true
print isFalse;  // false
```

### Number

64-bit floating-point numbers:

```volt
let integer = 42;
let float = 3.14159;
let negative = -17;
let scientific = 1.5e-3;  // 0.0015
```

### String

Immutable sequences of characters:

```volt
let singleQuote = 'Hello';
let doubleQuote = "World";
let multiline = "This is a
multiline string";

// String concatenation
let greeting = "Hello" + " " + "World";  // "Hello World"
```

### Array

Ordered collections of values:

```volt
let emptyArray = [];
let numbers = [1, 2, 3, 4, 5];
let mixed = [42, "hello", true, nil];
let nested = [[1, 2], [3, 4]];
```

### Hash Map

Key-value collections:

```volt
let emptyMap = {};
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};
let nested = {
    "user": {
        "id": 1,
        "name": "Bob"
    },
    "settings": {
        "theme": "dark",
        "notifications": true
    }
};
```

### Function

Callable objects:

```volt
let add = fun(a, b) {
    return a + b;
};

// Functions are first-class values
let operation = add;
let result = operation(5, 3);  // 8
```

---

## 🔀 Control Flow

### Conditional Statements

#### If Statement

```volt
let score = 85;

if (score >= 90) {
    print "Excellent!";
} else if (score >= 80) {
    print "Good job!";
} else if (score >= 70) {
    print "Passing";
} else {
    print "Needs improvement";
}
```

#### Ternary Operator

```volt
let age = 20;
let status = (age >= 18) ? "Adult" : "Minor";
print status;  // "Adult"
```

### Loops

#### While Loop

```volt
let i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
// Output: 0 1 2 3 4
```

#### For Loop

```volt
// Traditional for loop
for (let i = 0; i < 5; i = i + 1) {
    print i;
}

// For-in loop (iterate over array indices)
let numbers = [10, 20, 30, 40];
for (let i in numbers) {
    print numbers[i];
}
```

#### Run-Until Loop

Executes at least once, continues until condition becomes true:

```volt
let count = 5;
run {
    print count;
    count = count - 1;
} until (count <= 0);
// Output: 5 4 3 2 1
```

### Loop Control

#### Break Statement

```volt
for (let i = 0; i < 10; i = i + 1) {
    if (i == 5) {
        break;  // Exit the loop
    }
    print i;
}
// Output: 0 1 2 3 4
```

#### Continue Statement

```volt
for (let i = 0; i < 10; i = i + 1) {
    if (i % 2 == 0) {
        continue;  // Skip even numbers
    }
    print i;
}
// Output: 1 3 5 7 9
```

---

## 🔄 Functions

### Function Declaration

```volt
fn greet(name) {
    return "Hello, " + name + "!";
}

print greet("Alice");  // "Hello, Alice!"
```

### Parameters and Return Values

```volt
fn add(a, b) {
    return a + b;
}

fn multiply(a, b) {
    return a * b;
}

// Functions without explicit return return nil
fn printTwice(message) {
    print message;
    print message;
    // Implicitly returns nil
}
```

### Early Return

```volt
fn findFirstEven(numbers) {
    for (let i = 0; i < numbers.length; i = i + 1) {
        if (numbers[i] % 2 == 0) {
            return numbers[i];  // Exit function immediately
        }
    }
    return nil;  // No even number found
}

print findFirstEven([1, 3, 4, 7, 8]);  // 4
```

### First-Class Functions

Functions are values that can be assigned and passed around:

```volt
// Assign function to variable
let operation = fun(x, y) {
    return x * y;
};

print operation(5, 3);  // 15

// Pass function as argument
fn apply(func, a, b) {
    return func(a, b);
}

let result = apply(operation, 4, 7);  // 28
```

### Closures

Functions that capture variables from their surrounding scope:

```volt
fn makeCounter() {
    let count = 0;
    
    return fun() {
        count = count + 1;
        return count;
    };
}

let counter = makeCounter();
print counter();  // 1
print counter();  // 2
print counter();  // 3
```

### Higher-Order Functions

Functions that take other functions as parameters or return functions:

```volt
// Function that returns a function
fn makeAdder(x) {
    return fun(y) {
        return x + y;
    };
}

let addFive = makeAdder(5);
print addFive(3);  // 8
print addFive(10); // 15

// Function that takes a function as parameter
fn forEach(array, callback) {
    for (let i = 0; i < array.length; i = i + 1) {
        callback(array[i]);
    }
}

let numbers = [1, 2, 3, 4, 5];
forEach(numbers, fun(item) {
    print item * 2;
});
// Output: 2 4 6 8 10
```

### Recursion

Functions can call themselves:

```volt
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print factorial(5);  // 120

fn fibonacci(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

print fibonacci(8);  // 21
```

---

## 📦 Arrays

### Creating Arrays

```volt
// Empty array
let empty = [];

// Array with elements
let numbers = [1, 2, 3, 4, 5];

// Mixed types
let mixed = [42, "hello", true, nil, [1, 2, 3]];

// Nested arrays
let matrix = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
];

// Trailing comma (allowed)
let items = [
    "apple",
    "banana",
    "cherry",
];
```

### Accessing Elements

```volt
let fruits = ["apple", "banana", "cherry"];

// Get element by index
print fruits[0];  // "apple"
print fruits[1];  // "banana"
print fruits[2];  // "cherry"

// Get nested elements
let matrix = [[1, 2], [3, 4]];
print matrix[1][0];  // 3

// Get length
print fruits.length;  // 3
```

### Modifying Arrays

```volt
let numbers = [1, 2, 3, 4, 5];

// Change element
numbers[0] = 10;
print numbers[0];  // 10

// Add element to end
numbers.push(6);
print numbers.length;  // 6

// Remove last element
let last = numbers.pop();
print last;           // 6
print numbers.length; // 5

// Reverse array in place
numbers.reverse();
print numbers[0];  // 5
```

### Array Methods

#### Push

```volt
let arr = [1, 2, 3];
arr.push(4);
print arr;  // [1, 2, 3, 4]
```

#### Pop

```volt
let arr = [1, 2, 3];
let last = arr.pop();
print last;  // 3
print arr;   // [1, 2]
```

#### Reverse

```volt
let arr = [1, 2, 3, 4];
arr.reverse();
print arr;  // [4, 3, 2, 1]
```

### Iterating Over Arrays

```volt
let numbers = [10, 20, 30, 40, 50];

// Traditional for loop
for (let i = 0; i < numbers.length; i = i + 1) {
    print numbers[i];
}

// For-in loop
for (let i in numbers) {
    print numbers[i];
}

// Using forEach (higher-order function)
fn forEach(array, callback) {
    for (let i = 0; i < array.length; i = i + 1) {
        callback(array[i]);
    }
}

forEach(numbers, fun(item) {
    print "Number: " + str(item);
});
```

### Common Array Operations

#### Finding Maximum

```volt
fn max(array) {
    if (array.length == 0) return nil;
    
    let maxValue = array[0];
    for (let i = 1; i < array.length; i = i + 1) {
        if (array[i] > maxValue) {
            maxValue = array[i];
        }
    }
    return maxValue;
}

print max([3, 7, 2, 9, 1]);  // 9
```

#### Summing Elements

```volt
fn sum(array) {
    let total = 0;
    for (let i = 0; i < array.length; i = i + 1) {
        total = total + array[i];
    }
    return total;
}

print sum([1, 2, 3, 4, 5]);  // 15
```

#### Filtering Elements

```volt
fn filter(array, predicate) {
    let result = [];
    for (let i = 0; i < array.length; i = i + 1) {
        if (predicate(array[i])) {
            result.push(array[i]);
        }
    }
    return result;
}

let numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
let evens = filter(numbers, fun(x) { return x % 2 == 0; });
print evens;  // [2, 4, 6, 8, 10]
```

#### Mapping Elements

```volt
fn map(array, transformer) {
    let result = [];
    for (let i = 0; i < array.length; i = i + 1) {
        result.push(transformer(array[i]));
    }
    return result;
}

let numbers = [1, 2, 3, 4, 5];
let doubled = map(numbers, fun(x) { return x * 2; });
print doubled;  // [2, 4, 6, 8, 10]
```

---

## 🗺️ Hash Maps

### Creating Hash Maps

```volt
// Empty hash map
let empty = {};

// Hash map with string keys
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

// Hash map with mixed key types (converted to strings)
let mixed = {
    "name": "Bob",
    42: "answer",
    true: "boolean key"
};

// Nested hash maps
let company = {
    "name": "Tech Corp",
    "departments": {
        "engineering": {
            "head": "Alice",
            "size": 10
        },
        "marketing": {
            "head": "Bob",
            "size": 5
        }
    }
};
```

### Accessing Values

```volt
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

// Get value by key
print person["name"];  // "Alice"
print person["age"];   // 25

// Get nested values
let company = {
    "departments": {
        "engineering": {
            "head": "Alice"
        }
    }
};
print company["departments"]["engineering"]["head"];  // "Alice"

// Get size
print person.size;  // 3
```

### Modifying Hash Maps

```volt
let person = {
    "name": "Alice",
    "age": 25
};

// Add new key-value pair
person["email"] = "alice@example.com";
print person["email"];  // "alice@example.com"

// Update existing value
person["age"] = 26;
print person["age"];  // 26

// Remove key-value pair
let wasRemoved = remove(person, "age");
print wasRemoved;     // true
print has(person, "age");  // false
```

### Hash Map Methods

#### Keys

```volt
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

let keys = keys(person);
print keys;  // ["name", "age", "active"]
```

#### Values

```volt
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

let values = values(person);
print values;  // ["Alice", 25, true]
```

#### Has

```volt
let person = {
    "name": "Alice",
    "age": 25
};

print has(person, "name");    // true
print has(person, "email");   // false
print has(person, "age");     // true
```

#### Remove

```volt
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

let wasRemoved = remove(person, "age");
print wasRemoved;  // true
print has(person, "age");  // false

let notFound = remove(person, "salary");
print notFound;  // false
```

### Iterating Over Hash Maps

```volt
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

// Get keys and iterate
let keysList = keys(person);
for (let i = 0; i < keysList.length; i = i + 1) {
    let key = keysList[i];
    let value = person[key];
    print key + ": " + str(value);
}

// Using forEach for key-value pairs
fn forEachKeyValue(map, callback) {
    let keyList = keys(map);
    for (let i = 0; i < keyList.length; i = i + 1) {
        let key = keyList[i];
        callback(key, map[key]);
    }
}

forEachKeyValue(person, fun(key, value) {
    print key + " = " + str(value);
});
```

### Common Hash Map Operations

#### Counting Word Frequencies

```volt
fn wordCount(text) {
    let words = split(text, " ");
    let counts = {};
    
    for (let i = 0; i < words.length; i = i + 1) {
        let word = trim(words[i]);
        if (word != "") {
            if (has(counts, word)) {
                counts[word] = counts[word] + 1;
            } else {
                counts[word] = 1;
            }
        }
    }
    
    return counts;
}

let text = "the quick brown fox jumps over the lazy dog the fox";
let frequencies = wordCount(text);
print frequencies["the"];   // 3
print frequencies["fox"];   // 2
print frequencies["quick"]; // 1
```

#### Grouping Data

```volt
fn groupBy(array, keyFunction) {
    let groups = {};
    
    for (let i = 0; i < array.length; i = i + 1) {
        let item = array[i];
        let key = keyFunction(item);
        
        if (!has(groups, key)) {
            groups[key] = [];
        }
        groups[key].push(item);
    }
    
    return groups;
}

let people = [
    {"name": "Alice", "department": "Engineering"},
    {"name": "Bob", "department": "Marketing"},
    {"name": "Carol", "department": "Engineering"},
    {"name": "Dave", "department": "Sales"}
];

let byDepartment = groupBy(people, fun(person) { 
    return person["department"]; 
});

print byDepartment["Engineering"].length;  // 2
print byDepartment["Marketing"].length;    // 1
print byDepartment["Sales"].length;        // 1
```

---

## 📁 File I/O

### Reading Files

```volt
// Read entire file as string
let content = readFile("data.txt");
print content;

// Handle file not found
if (fileExists("data.txt")) {
    let content = readFile("data.txt");
    print "File content: " + content;
} else {
    print "File not found!";
}
```

### Writing Files

```volt
// Write string to file (overwrites existing content)
let success = writeFile("output.txt", "Hello, World!");
if (success) {
    print "File written successfully";
} else {
    print "Failed to write file";
}

// Append to file
let appendSuccess = appendFile("log.txt", "New log entry\n");
if (appendSuccess) {
    print "Entry appended";
}
```

### File Operations

```volt
// Check if file exists
if (exists("config.json")) {
    print "Configuration file exists";
    print "File size: " + str(fileSize("config.json")) + " bytes";
} else {
    print "Configuration file not found";
}

// Delete file
if (exists("temp.txt")) {
    let deleted = deleteFile("temp.txt");
    if (deleted) {
        print "Temporary file deleted";
    } else {
        print "Failed to delete file";
    }
}
```

### Practical Example: Configuration Management

```volt
// Read configuration
fn loadConfig(filename) {
    if (!exists(filename)) {
        // Create default configuration
        let defaultConfig = {
            "appName": "MyApp",
            "version": "1.0.0",
            "debug": true,
            "maxUsers": 100
        };
        saveConfig(filename, defaultConfig);
        return defaultConfig;
    }
    
    let content = readFile(filename);
    return jsonDecode(content);
}

// Save configuration
fn saveConfig(filename, config) {
    let jsonContent = jsonEncode(config);
    return writeFile(filename, jsonContent);
}

// Usage
let config = loadConfig("app.config");
print "App: " + config["appName"];
print "Version: " + config["version"];

// Update configuration
config["maxUsers"] = 150;
saveConfig("app.config", config);
```

---

## 🧵 String Operations

### Basic String Operations

```volt
let text = "Hello, World!";

// Get length
print len(text);  // 13

// Convert case
print toUpper(text);  // "HELLO, WORLD!"
print toLower(text);  // "hello, world!"
print upper(text);    // "HELLO, WORLD!" (alias)
print lower(text);    // "hello, world!" (alias)

// Extract substring
print substr(text, 0, 5);    // "Hello"
print substr(text, 7, 5);    // "World"

// Find substring
print indexOf(text, "World");  // 7
print indexOf(text, "xyz");    // -1 (not found)
```

### String Manipulation

```volt
// Trim whitespace
let spaced = "   Hello, World!   ";
print "'" + trim(spaced) + "'";  // "'Hello, World!'"

// Split string
let csv = "apple,banana,cherry,date";
let fruits = split(csv, ",");
print fruits[0];  // "apple"
print fruits[1];  // "banana"

// Split into characters
let word = "hello";
let chars = split(word, "");
print chars[0];  // "h"
print chars[1];  // "e"

// Replace text
let sentence = "The quick brown fox";
let modified = replace(sentence, "fox", "cat");
print modified;  // "The quick brown cat"

// Replace all occurrences
let text = "hello hello hello";
let replaced = replace(text, "hello", "hi");
print replaced;  // "hi hi hi"
```

### String Comparison

```volt
let str1 = "Hello";
let str2 = "World";

// Check prefixes and suffixes
print startsWith(str1, "Hel");    // true
print startsWith(str1, "Wor");    // false
print endsWith(str2, "rld");      // true
print endsWith(str2, "abc");      // false
```

### Advanced String Functions

```volt
// Padding
let number = "42";
print "'" + padStart(number, 5, "0") + "'";  // "'00042'"
print "'" + padEnd(number, 5, "0") + "'";    // "'42000'"

// Repeat string
print repeat("Hi ", 3);  // "Hi Hi Hi "
print repeat("*", 10);   // "**********"

// Character codes
print charCodeAt("A", 0);     // 65
print charCodeAt("ABC", 1);   // 66
print fromCharCode(65);       // "A"
print fromCharCode(66);       // "B"
```

### String Processing Examples

#### CSV Parsing

```volt
fn parseCSV(line) {
    let fields = split(line, ",");
    let result = [];
    
    for (let i = 0; i < fields.length; i = i + 1) {
        result.push(trim(fields[i]));
    }
    
    return result;
}

let csvLine = "John Doe, 25, Engineer, New York";
let fields = parseCSV(csvLine);
print "Name: " + fields[0];
print "Age: " + fields[1];
print "Job: " + fields[2];
print "City: " + fields[3];
```

#### Text Analysis

```volt
fn analyzeText(text) {
    let cleanText = toLower(trim(text));
    let words = split(cleanText, " ");
    
    let wordCount = 0;
    let charCount = 0;
    
    for (let i = 0; i < words.length; i = i + 1) {
        let word = trim(words[i]);
        if (len(word) > 0) {
            wordCount = wordCount + 1;
            charCount = charCount + len(word);
        }
    }
    
    return {
        "wordCount": wordCount,
        "charCount": charCount,
        "avgWordLength": charCount / wordCount
    };
}

let text = "The quick brown fox jumps over the lazy dog";
let analysis = analyzeText(text);
print "Words: " + str(analysis["wordCount"]);
print "Characters: " + str(analysis["charCount"]);
print "Average word length: " + str(analysis["avgWordLength"]);
```

---

## 🧮 Mathematical Functions

### Basic Math Functions

```volt
// Absolute value
print abs(-5);    // 5
print abs(3.14);  // 3.14

// Square root
print sqrt(16);   // 4
print sqrt(2);    // 1.4142135623730951

// Power function
print pow(2, 3);  // 8
print pow(5, 2);  // 25

// Minimum and maximum
print min(10, 5);  // 5
print max(10, 5);  // 10

// Rounding
print round(3.7);   // 4
print round(3.2);   // 3
print floor(3.7);   // 3
print ceil(3.2);    // 4

// Random number
print random();     // Random number between 0 and 1
```

### Trigonometric Functions

```volt
// Convert degrees to radians for trig functions
fn degToRad(degrees) {
    return degrees * 3.14159 / 180;
}

let angle = degToRad(45);
print sin(angle);  // 0.7071067811865475
print cos(angle);  // 0.7071067811865476
print tan(angle);  // 0.9999999999999999
```

### Logarithmic Functions

```volt
// Natural logarithm
print log(1);      // 0
print log(2.71828); // ~1

// Exponential function
print exp(0);      // 1
print exp(1);      // 2.718281828459045
```

### Practical Math Examples

#### Distance Calculation

```volt
fn distance(x1, y1, x2, y2) {
    let dx = x2 - x1;
    let dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

print distance(0, 0, 3, 4);  // 5
```

#### Quadratic Formula

```volt
fn solveQuadratic(a, b, c) {
    let discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return nil;  // No real solutions
    }
    
    let sqrtDisc = sqrt(discriminant);
    let root1 = (-b + sqrtDisc) / (2 * a);
    let root2 = (-b - sqrtDisc) / (2 * a);
    
    return [root1, root2];
}

let solutions = solveQuadratic(1, -5, 6);
print solutions[0];  // 3
print solutions[1];  // 2
```

#### Compound Interest

```volt
fn compoundInterest(principal, rate, time) {
    return principal * pow(1 + rate, time);
}

let amount = compoundInterest(1000, 0.05, 10);
print "Final amount: $" + str(amount);  // $1628.89
```

---

## 🔁 Functional Programming

### Function Composition

```volt
// Basic composition
let add = fun(x, y) { return x + y; };
let multiply = fun(x, y) { return x * y; };
let square = fun(x) { return x * x; };

// Compose functions (right to left)
let addThenSquare = compose(square, add);
let result = addThenSquare(3, 4);  // square(add(3, 4)) = square(7) = 49
print result;

// Multiple function composition
let process = compose(square, multiply, add);
let result2 = process(2, 3, 4);  // square(multiply(add(2, 3), 4)) = square(multiply(5, 4)) = square(20) = 400
print result2;
```

### Function Piping

```volt
// Pipe functions (left to right)
let addOne = fun(x) { return x + 1; };
let double = fun(x) { return x * 2; };
let square = fun(x) { return x * x; };

let piped = pipe(addOne, double, square);
let result = piped(5);  // square(double(addOne(5))) = square(double(6)) = square(12) = 144
print result;

// Practical data processing pipeline
let numbers = [1, 2, 3, 4, 5];

let processPipeline = pipe(
    fun(arr) { return map(arr, fun(x) { return x * 2; }); },  // Double each number
    fun(arr) { return filter(arr, fun(x) { return x > 5; }); },  // Keep numbers > 5
    fun(arr) { return reduce(arr, fun(acc, x) { return acc + x; }, 0); }  // Sum remaining numbers
);

let result = processPipeline(numbers);
print result;  // 18 (6 + 8 + 10)
```

### Higher-Order Functions

```volt
// Map function
fn map(array, func) {
    let result = [];
    for (let i = 0; i < array.length; i = i + 1) {
        result.push(func(array[i]));
    }
    return result;
}

// Filter function
fn filter(array, predicate) {
    let result = [];
    for (let i = 0; i < array.length; i = i + 1) {
        if (predicate(array[i])) {
            result.push(array[i]);
        }
    }
    return result;
}

// Reduce function
fn reduce(array, reducer, initialValue) {
    let accumulator = initialValue;
    for (let i = 0; i < array.length; i = i + 1) {
        accumulator = reducer(accumulator, array[i]);
    }
    return accumulator;
}

// Usage examples
let numbers = [1, 2, 3, 4, 5];

let doubled = map(numbers, fun(x) { return x * 2; });
print doubled;  // [2, 4, 6, 8, 10]

let evens = filter(numbers, fun(x) { return x % 2 == 0; });
print evens;  // [2, 4]

let sum = reduce(numbers, fun(acc, x) { return acc + x; }, 0);
print sum;  // 15

let product = reduce(numbers, fun(acc, x) { return acc * x; }, 1);
print product;  // 120
```

### Function Factories

```volt
// Create multiplier functions
fn createMultiplier(factor) {
    return fun(value) {
        return value * factor;
    };
}

let doubler = createMultiplier(2);
let tripler = createMultiplier(3);
let halfer = createMultiplier(0.5);

print doubler(10);   // 20
print tripler(10);   // 30
print halfer(10);    // 5

// Create validator functions
fn createValidator(min, max) {
    return fun(value) {
        return value >= min && value <= max;
    };
}

let isAdult = createValidator(18, 120);
let isChild = createValidator(0, 17);

print isAdult(25);   // true
print isAdult(15);   // false
print isChild(10);   // true
```

### Currying

```volt
// Manual currying
fn add(a) {
    return fun(b) {
        return fun(c) {
            return a + b + c;
        };
    };
}

let addFive = add(5);
let addFiveAndThree = addFive(3);
let result = addFiveAndThree(2);  // 5 + 3 + 2 = 10
print result;

// Generic curry function
fn curry(func, arity) {
    return fun(...args) {
        if (args.length >= arity) {
            return func(...args);
        } else {
            return curry(fun(...moreArgs) {
                return func(...args, ...moreArgs);
            }, arity - args.length);
        }
    };
}

let curriedAdd = curry(fun(a, b, c) { return a + b + c; }, 3);
print curriedAdd(1)(2)(3);        // 6
print curriedAdd(1, 2)(3);        // 6
print curriedAdd(1)(2, 3);        // 6
print curriedAdd(1, 2, 3);        // 6
```

---

## ⚡ Performance Tools

### Sleep Function

```volt
// Pause execution for specified milliseconds
print "Starting...";
sleep(1000);  // Sleep for 1 second
print "1 second later!";

// Create delays between operations
for (let i = 5; i > 0; i = i - 1) {
    print i;
    sleep(500);  // Half second delay
}
print "Done!";
```

### Benchmarking

```volt
// Benchmark a function
fn slowFunction(n) {
    let result = 0;
    for (let i = 0; i < n; i = i + 1) {
        for (let j = 0; j < n; j = j + 1) {
            result = result + 1;
        }
    }
    return result;
}

fn fastFunction(n) {
    return n * n;
}

// Benchmark both functions
let slowResult = benchmark(slowFunction, 100);
let fastResult = benchmark(fastFunction, 100);

print "Slow function result: " + str(slowResult.result);
print "Slow function time: " + str(slowResult.timeMicroseconds) + " μs";

print "Fast function result: " + str(fastResult.result);
print "Fast function time: " + str(fastResult.timeMicroseconds) + " μs";

// Calculate speedup
if (fastResult.timeMicroseconds > 0) {
    let speedup = slowResult.timeMicroseconds / fastResult.timeMicroseconds;
    print "Speedup factor: " + str(speedup) + "x";
}
```

### Performance Comparison

```volt
// Compare different algorithms
fn bubbleSort(arr) {
    let n = arr.length;
    for (let i = 0; i < n; i = i + 1) {
        for (let j = 0; j < n - i - 1; j = j + 1) {
            if (arr[j] > arr[j + 1]) {
                let temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    return arr;
}

fn quickSort(arr) {
    if (arr.length <= 1) return arr;
    
    let pivot = arr[0];
    let left = [];
    let right = [];
    
    for (let i = 1; i < arr.length; i = i + 1) {
        if (arr[i] < pivot) {
            left.push(arr[i]);
        } else {
            right.push(arr[i]);
        }
    }
    
    return quickSort(left).concat([pivot]).concat(quickSort(right));
}

// Generate test data
fn generateRandomArray(size) {
    let arr = [];
    for (let i = 0; i < size; i = i + 1) {
        arr.push(floor(random() * 1000));
    }
    return arr;
}

// Benchmark sorting algorithms
let testData = generateRandomArray(100);

let bubbleTime = benchmark(bubbleSort, [testData]);
let quickTime = benchmark(quickSort, [testData]);

print "Bubble sort: " + str(bubbleTime.timeMicroseconds) + " μs";
print "Quick sort: " + str(quickTime.timeMicroseconds) + " μs";

if (quickTime.timeMicroseconds > 0) {
    let speedup = bubbleTime.timeMicroseconds / quickTime.timeMicroseconds;
    print "Quick sort is " + str(speedup) + "x faster";
}
```

### Timing Operations

```volt
// Time a block of code
fn timeOperation(name, operation) {
    print "Starting " + name + "...";
    let start = now();
    
    let result = operation();
    
    let end = now();
    let elapsed = end - start;
    
    print name + " completed in " + str(elapsed) + " ms";
    return result;
}

// Usage
let result = timeOperation("calculation", fun() {
    let sum = 0;
    for (let i = 0; i < 1000000; i = i + 1) {
        sum = sum + i;
    }
    return sum;
});

print "Result: " + str(result);
```

---

## ⚠️ Error Handling

### Runtime Errors

VoltScript provides precise error reporting with line and column information:

```volt
// Division by zero
let result = 10 / 0;  // Error: Division by zero

// Array index out of bounds
let arr = [1, 2, 3];
print arr[10];  // Error: Array index out of bounds: 10

// Invalid operation
let str = "hello";
let result = str - 5;  // Error: Operands must be numbers

// Function call on non-function
let notAFunction = 42;
notAFunction();  // Error: Can only call functions and classes
```

### Error Information

Errors include:
- **Error message**: Description of what went wrong
- **Line number**: Where the error occurred
- **Column number**: Exact position in the line
- **Token information**: The specific token that caused the error

### Best Practices for Error Prevention

```volt
// Check array bounds before access
fn safeGet(array, index) {
    if (index >= 0 && index < array.length) {
        return array[index];
    } else {
        return nil;
    }
}

let arr = [1, 2, 3];
print safeGet(arr, 1);  // 2
print safeGet(arr, 10); // nil

// Check file existence before reading
fn safeReadFile(filename) {
    if (fileExists(filename)) {
        return readFile(filename);
    } else {
        return "File not found: " + filename;
    }
}

let content = safeReadFile("data.txt");
print content;

// Validate input types
fn addNumbers(a, b) {
    if (type(a) != "number" || type(b) != "number") {
        throw "Both arguments must be numbers";
    }
    return a + b;
}

try {
    let result = addNumbers(5, "hello");
} catch (error) {
    print "Error: " + error;
}
```

---

## 📚 Built-in Functions

### Type Conversion Functions

```volt
// Convert to string
print str(42);        // "42"
print str(true);      // "true"
print str(nil);       // "nil"

// Convert to number
print num("123");     // 123
print num("3.14");    // 3.14
print num(true);      // 1
print num(false);     // 0

// Get type as string
print type(42);       // "number"
print type("hello");  // "string"
print type(true);     // "bool"
print type(nil);      // "nil"
print type([1,2,3]);  // "array"
print type({"a":1});  // "hashmap"
print type(fun(x){}); // "function"
```

### Input/Output Functions

```volt
// Print to console
print "Hello, World!";
print 42;
print [1, 2, 3];

// Read input from user
let name = input("Enter your name: ");
print "Hello, " + name + "!";

let age = input("Enter your age: ");
let ageNum = num(age);
print "Next year you'll be " + str(ageNum + 1);
```

### Utility Functions

```volt
// Get current time
let timestamp = now();
print "Current timestamp: " + str(timestamp);

// Format date (stub implementation)
let formatted = formatDate(now(), "YYYY-MM-DD");
print "Formatted date: " + formatted;

// JSON encoding/decoding
let obj = {
    "name": "Alice",
    "age": 25,
    "active": true
};

let jsonString = jsonEncode(obj);
print jsonString;

let parsed = jsonDecode(jsonString);
print parsed["name"];  // "Alice"
```

---

## 🎯 Best Practices

### Naming Conventions

```volt
// Use descriptive names
let userAge = 25;           // Good
let ua = 25;                // Avoid

let calculateTotal = fun(prices) {  // Good
    // implementation
};

let calc = fun(p) {         // Avoid
    // implementation
};
```

### Code Organization

```volt
// Group related functionality
fn mathUtils = {
    "add": fun(a, b) { return a + b; },
    "subtract": fun(a, b) { return a - b; },
    "multiply": fun(a, b) { return a * b; }
};

// Use consistent indentation
if (condition) {
    doSomething();
    doAnotherThing();
} else {
    handleAlternative();
}
```

### Error Prevention

```volt
// Validate inputs
fn divide(a, b) {
    if (type(a) != "number" || type(b) != "number") {
        throw "Arguments must be numbers";
    }
    if (b == 0) {
        throw "Division by zero";
    }
    return a / b;
}

// Check array bounds
fn getLast(array) {
    if (array.length == 0) {
        return nil;
    }
    return array[array.length - 1];
}
```

### Performance Considerations

```volt
// Avoid unnecessary operations in loops
// Bad:
for (let i = 0; i < array.length; i = i + 1) {
    let length = array.length;  // Calculated every iteration
    // use length
}

// Good:
let length = array.length;
for (let i = 0; i < length; i = i + 1) {
    // use length
}

// Use appropriate data structures
// For frequent lookups, use hash maps
let lookup = {
    "key1": "value1",
    "key2": "value2"
};
print lookup["key1"];  // Fast O(1) lookup

// For ordered data, use arrays
let ordered = [1, 2, 3, 4, 5];
// Process in order
```

---

## 📖 Examples

### Complete Programs

#### Calculator

```volt
fn calculator() {
    print "=== Simple Calculator ===";
    
    let num1 = num(input("Enter first number: "));
    let operator = input("Enter operator (+, -, *, /): ");
    let num2 = num(input("Enter second number: "));
    
    let result;
    
    if (operator == "+") {
        result = num1 + num2;
    } else if (operator == "-") {
        result = num1 - num2;
    } else if (operator == "*") {
        result = num1 * num2;
    } else if (operator == "/") {
        if (num2 != 0) {
            result = num1 / num2;
        } else {
            print "Error: Division by zero";
            return;
        }
    } else {
        print "Error: Invalid operator";
        return;
    }
    
    print "Result: " + str(result);
}

calculator();
```

#### Todo List Manager

```volt
fn todoManager() {
    let todos = [];
    
    while (true) {
        print "\n=== Todo Manager ===";
        print "1. Add todo";
        print "2. List todos";
        print "3. Remove todo";
        print "4. Exit";
        
        let choice = num(input("Enter choice: "));
        
        if (choice == 1) {
            let todo = input("Enter todo: ");
            todos.push(todo);
            print "Todo added!";
        } else if (choice == 2) {
            if (todos.length == 0) {
                print "No todos yet!";
            } else {
                print "Todos:";
                for (let i = 0; i < todos.length; i = i + 1) {
                    print (i + 1) + ". " + todos[i];
                }
            }
        } else if (choice == 3) {
            if (todos.length == 0) {
                print "No todos to remove!";
            } else {
                let index = num(input("Enter todo number to remove: ")) - 1;
                if (index >= 0 && index < todos.length) {
                    let removed = todos.pop(index);
                    print "Removed: " + removed;
                } else {
                    print "Invalid todo number!";
                }
            }
        } else if (choice == 4) {
            print "Goodbye!";
            break;
        } else {
            print "Invalid choice!";
        }
    }
}

todoManager();
```

#### File Processor

```volt
fn processFile(filename) {
    if (!exists(filename)) {
        print "File not found: " + filename;
        return;
    }
    
    let content = readFile(filename);
    let lines = split(content, "\n");
    
    let wordCount = 0;
    let lineCount = 0;
    
    for (let i = 0; i < lines.length; i = i + 1) {
        let line = trim(lines[i]);
        if (len(line) > 0) {
            lineCount = lineCount + 1;
            let words = split(line, " ");
            for (let j = 0; j < words.length; j = j + 1) {
                if (len(trim(words[j])) > 0) {
                    wordCount = wordCount + 1;
                }
            }
        }
    }
    
    print "File: " + filename;
    print "Lines: " + str(lineCount);
    print "Words: " + str(wordCount);
    print "Characters: " + str(len(content));
}

processFile("document.txt");
```

---

## ❓ FAQ

### General Questions

**Q: What is VoltScript?**
A: VoltScript is a dynamically-typed programming language built from scratch in C++20. It's designed to be both educational and practical.

**Q: Why build a language from scratch?**
A: To understand how programming languages work internally - lexers, parsers, interpreters, and runtime systems.

**Q: Is VoltScript production-ready?**
A: While functional and well-tested, it's primarily educational. For production use, consider established languages.

### Technical Questions

**Q: What C++ standard is required?**
A: C++20 or higher is required for modern features and standard library support.

**Q: How fast is VoltScript?**
A: As a tree-walk interpreter, it's slower than compiled languages but suitable for learning and small scripts.

**Q: Does it have garbage collection?**
A: It uses C++ smart pointers (shared_ptr) for automatic memory management.

### Language Features

**Q: Does it support object-oriented programming?**
A: Basic object support through hash maps. Full OOP support is planned for future versions.

**Q: Are there modules or imports?**
A: Not yet. All code must be in a single file or manually concatenated.

**Q: What data structures are available?**
A: Arrays, hash maps, strings, numbers, booleans, and functions.

### Development

**Q: How many tests does it have?**
A: Over 450 comprehensive unit tests covering all language features.

**Q: How is error reporting handled?**
A: Precise error messages with line and column numbers for exact source location.

**Q: Can I contribute?**
A: Yes! Contributions are welcome for bug fixes, features, and documentation improvements.

---

## 📄 License

MIT License - Free to use, modify, and distribute.

---

<div align="center">

### ⚡ VoltScript v0.7.9 ⚡

**Built with passion and C++20**

[GitHub Repository](https://github.com/yourusername/voltscript) • [Report Issues](https://github.com/yourusername/voltscript/issues)

</div>