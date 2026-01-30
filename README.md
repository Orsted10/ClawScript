<div align="center">

# ⚡ VoltScript v0.7.9 ⚡
### *A programming language built from scratch in C++20*

**Production-Ready • Feature-Rich • Educational**  

> *From tokens → trees → closures → arrays → objects*  

[![Tests](https://img.shields.io/badge/tests-650%2B-brightgreen)]()
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

</div>

---

## 🧠 What is VoltScript?

VoltScript is a **fully-functional programming language** written completely from scratch in **C++20**.

Not a transpiler.  
Not a wrapper.  
Not a toy.

This project answers one question honestly:
> **How do programming languages actually work under the hood?**

So instead of hiding complexity, VoltScript **embraces it**—and builds every layer explicitly:

- ⚡ Lexical analysis
- 🌳 Parsing with recursive descent  
- 🧠 AST construction
- ⚙️ Tree-walk interpretation
- 📍 Precise error reporting
- 🔐 Lexical scoping & environments
- 🔁 First-class functions & closures
- 📦 Dynamic arrays with methods
- 🎯 Compound operators & control flow

If you've ever wanted to *truly* understand languages like **Python**, **JavaScript**, or **Lua** — this is your path.

---

## ✨ Features (v0.7.9)

### 🧩 Lexer — *Characters → Tokens*

- **Literals**: Numbers (`42`, `3.14`), Strings (`"hello"`), Booleans (`true`, `false`), `nil`
- **Keywords**: `let`, `if`, `else`, `while`, `for`, `run`, `until`, `fn`, `return`, `print`, `break`, `continue`
- **Operators**: Arithmetic, Comparison, Logical, Assignment, Compound (`+=`, `-=`, `*=`, `/=`), Increment/Decrement (`++`, `--`), Ternary (`? :`)
- **Data Structures**: Arrays `[]`, Hash Maps `{}`
- **Comments**: `// line comments`

### ⚙️ Interpreter — *AST → Execution*

VoltScript runs **real programs** with:

- ✅ Arithmetic with correct precedence
- ✅ Variables with `let` + reassignment
- ✅ Block scoping & shadowing
- ✅ Control flow: `if`/`else`, `while`, `for`, `run-until`
- ✅ **Loop control**: `break` and `continue`
- ✅ **Compound assignment**: `x += 5`, `s *= 2`
- ✅ **Increment/Decrement**: `++i`, `i--`, etc.
- ✅ **Ternary operator**: `max = (a > b) ? a : b;`
- ✅ **Precise error reporting**: Errors point to exact source tokens
- ✅ **Hash Map support**: `{ "key": "value", "nested": { "prop": 42 } }`
- ✅ **File I/O**: `readFile()`, `writeFile()`, `exists()`, `deleteFile()`, `fileSize()`
- ✅ **Advanced strings**: `padStart()`, `padEnd()`, `repeat()`, `charCodeAt()`, `fromCharCode()`
- ✅ **Functional utilities**: `compose()`, `pipe()` for function composition
- ✅ **Performance tools**: `sleep()`, `benchmark()` for timing

### 🔁 Functions & Closures

- ✅ `fn` declarations
- ✅ Parameters & return values
- ✅ Early `return`
- ✅ **First-class functions** (functions as values)
- ✅ **Closures** (capture surrounding scope)
- ✅ **Recursion**
- ✅ **Higher-order functions**
- ✅ **Native C++ functions** callable from VoltScript

### 📦 Arrays — *Dynamic Collections*

```volt
let numbers = [1, 2, 3, 4, 5];
let mixed = [42, "hello", true, nil];
let nested = [[1, 2], [3, 4]];

// Access
print numbers[0];          // 1
print nested[1][0];        // 3

// Modify
numbers[0] = 99;
print numbers[0];          // 99

// Properties & Methods
print numbers.length;      // 5
numbers.push(6);           // Add element
let last = numbers.pop();  // Remove and return last
numbers.reverse();         // In-place reverse
```

Array features:
- ✅ Dynamic sizing
- ✅ Mixed types
- ✅ Nested arrays
- ✅ Index access & assignment
- ✅ `.length` property
- ✅ `.push(value)` method
- ✅ `.pop()` method
- ✅ `.reverse()` method (in-place)

### 🗺️ Hash Maps — *Key-Value Collections*

```volt
let person = {
    "name": "Alice",
    "age": 25,
    "active": true,
    "scores": [95, 87, 92]
};

print person["name"];           // Access value
person["email"] = "alice@example.com";  // Dynamic addition

// Built-in functions
let keys = keys(person);        // Get all keys
let values = values(person);    // Get all values
print person.size;              // Get size (4)
print has(person, "name");      // Check key exists (true)
print remove(person, "age");    // Remove key-value pair
```

Hash Map features:
- ✅ Dynamic key-value pairs
- ✅ Mixed value types
- ✅ Nested hash maps
- ✅ Key access & assignment
- ✅ `.size` property
- ✅ `.keys()` method
- ✅ `.values()` method
- ✅ `.has(key)` method
- ✅ `.remove(key)` method

### 🧮 Built-in Functions

**Math Functions:**
- `abs(x)`, `sqrt(x)`, `pow(base, exp)`, `min(a,b)`, `max(a,b)`
- `round(x)`, `floor(x)`, `ceil(x)`, `random()`
- `sin(x)`, `cos(x)`, `tan(x)`, `log(x)`, `exp(x)`

**String Functions:**
- `len(x)`, `str(x)`, `num(x)`, `type(x)`
- `toUpper(s)`, `toLower(s)`, `upper(s)`, `lower(s)`
- `substr(s, start, length)`, `indexOf(s, substr)`
- `trim(s)`, `split(s, delimiter)`, `replace(s, search, replacement)`
- `startsWith(s, prefix)`, `endsWith(s, suffix)`
- `padStart(s, length, pad)`, `padEnd(s, length, pad)`
- `repeat(s, count)`, `charCodeAt(s, index)`, `fromCharCode(code)`

**File I/O:**
- `readFile(path)`, `writeFile(path, content)`, `appendFile(path, content)`
- `fileExists(path)`, `exists(path)`, `deleteFile(path)`, `fileSize(path)`

**Date/Time:**
- `now()` — Current timestamp in milliseconds
- `formatDate(timestamp, format)` — Format timestamp

**JSON:**
- `jsonEncode(value)` — Encode value to JSON string
- `jsonDecode(jsonString)` — Decode JSON string to value

**Functional Programming:**
- `compose(f1, f2, ...)` — Compose functions (right to left)
- `pipe(f1, f2, ...)` — Pipe value through functions (left to right)

**Performance:**
- `sleep(milliseconds)` — Pause execution
- `benchmark(func, ...args)` — Measure execution time

---

## 🛠️ Build & Run

### Requirements

* C++ compiler with **C++20 support** (MSVC 19.28+, GCC 10+, Clang 11+)
* CMake ≥ 3.14
* GoogleTest (automatically fetched)

### Windows (MSVC)

```bash
cmake -B build
cmake --build build --config Release
.\build\bin\Release\volt.exe
```

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/volt
```

### Run Tests

```bash
# Build and run all tests
cmake --build build --config Release
./build/bin/Release/volt_tests

# Or use CTest
ctest --output-on-failure --test-dir build
```

**Result:** ✅ **90% Pass Rate** (650+ tests, 584 passing)

---

## 💻 Using VoltScript

### Interactive REPL

```bash
⚡ VoltScript v0.7.9 REPL
Type 'exit' to quit
>> 
```

### Run a File

```bash
volt script.volt
```

---

## 📝 Code Examples

### 🎯 Arrays & Loops

```volt
// Build Fibonacci sequence
let fib = [0, 1];
for (let i = 0; i < 8; i++) {
    let next = fib[fib.length - 1] + fib[fib.length - 2];
    fib.push(next);
}
print fib;  // [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]
```

### 🔁 Array Functions

```volt
// Sum array elements
fn sum(arr) {
    let total = 0;
    for (let i = 0; i < arr.length; i++) {
        total += arr[i];
    }
    return total;
}

// Filter even numbers
fn filterEvens(arr) {
    let result = [];
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] % 2 == 0) {
            result.push(arr[i]);
        }
    }
    return result;
}

print sum([1, 2, 3, 4, 5]);           // 15
print filterEvens([1, 2, 3, 4, 5]);    // [2, 4]
```

### 🎲 Nested Arrays (Matrices)

```volt
let matrix = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
];

// Access elements
print matrix[1][2];  // 6

// Iterate
for (let i = 0; i < matrix.length; i++) {
    for (let j = 0; j < matrix[i].length; j++) {
        print matrix[i][j];
    }
}
```

### 🚦 Enhanced Control Flow

```volt
// Break and continue
for (let i = 1; i <= 10; i++) {
    if (i % 2 == 0) continue;  // Skip evens
    if (i > 7) break;          // Stop at 7
    print i;
}
// Output: 1 3 5 7

// Compound operators
let x = 10;
x += 5;   // x = 15
x *= 2;   // x = 30
x /= 3;   // x = 10

// Ternary operator
let max = (a > b) ? a : b;
let status = (score >= 60) ? "Pass" : "Fail";

// Run-until loop (executes at least once, until condition is true)
let countdown = 5;
run {
    print countdown;
    countdown--;
} until (countdown <= 0);
print "Liftoff!";
```

### 🗺️ Hash Maps

```volt
// Hash Maps - Key-value collections
let person = {
    "name": "Alice",
    "age": 25,
    "active": true,
    "scores": [95, 87, 92]
};

print person["name"];           // Access value
person["email"] = "alice@example.com";  // Dynamic addition

// Built-in functions for hash maps
let keys = keys(person);        // Get all keys
let values = values(person);    // Get all values
print person.size;              // Get size
print has(person, "name");      // Check key exists (true)
print remove(person, "age");    // Remove key-value pair

// Nested hash maps
let company = {
    "name": "Tech Corp",
    "departments": {
        "engineering": {"head": "Alice", "size": 10},
        "marketing": {"head": "Bob", "size": 5}
    }
};

print company["departments"]["engineering"]["head"];  // Alice
```

### 🔁 Closures with Arrays

```volt
fn makeStack() {
    let items = [];
    
    fn push(val) {
        items.push(val);
    }
    
    fn pop() {
        return items.pop();
    }
    
    fn size() {
        return items.length;
    }
    
    return [push, pop, size];
}

let stack = makeStack();
let push = stack[0];
let pop = stack[1];
let size = stack[2];

push(10);
push(20);
print pop();   // 20
print size();  // 1
```

### 🎯 Practical Example: Find Maximum

```volt
fn max(arr) {
    if (arr.length == 0) return nil;
    
    let maxVal = arr[0];
    for (let i = 1; i < arr.length; i++) {
        if (arr[i] > maxVal) {
            maxVal = arr[i];
        }
    }
    return maxVal;
}

print max([3, 7, 2, 9, 1]);  // 9
```

### 🎯 File Operations

```volt
// Check if file exists
if (exists("data.txt")) {
    print "File exists!";
    print "Size: " + str(fileSize("data.txt")) + " bytes";
} else {
    print "File not found";
    writeFile("data.txt", "Hello, VoltScript!");
}

// Read and process file
let content = readFile("data.txt");
print "Content: " + content;

// Append to file
appendFile("data.txt", "\nAppended line!");
```

### 🎯 Functional Programming

```volt
// Function composition
addOne = fun(x) { return x + 1; };
multiplyByTwo = fun(x) { return x * 2; };
square = fun(x) { return x * x; };

// Compose functions (right to left)
addThenSquare = compose(square, addOne);
result1 = addThenSquare(5);  // square(addOne(5)) = square(6) = 36

// Pipe functions (left to right)
squareThenDouble = pipe(square, multiplyByTwo);
result2 = squareThenDouble(3);  // multiplyByTwo(square(3)) = multiplyByTwo(9) = 18

print "Composed result: " + str(result1);
print "Piped result: " + str(result2);
```

### 🎯 Performance Benchmarking

```volt
// Benchmark different algorithms
slowFunction = fun(n) {
    result = 0;
    for (let i = 0; i < n; i = i + 1) {
        for (let j = 0; j < n; j = j + 1) {
            result = result + 1;
        }
    }
    return result;
};

fastFunction = fun(n) {
    return n * n;
};

print "Benchmarking...";
slowBench = benchmark(slowFunction, 100);
fastBench = benchmark(fastFunction, 100);

print "Slow function: " + str(slowBench.timeMicroseconds) + " μs";
print "Fast function: " + str(fastBench.timeMicroseconds) + " μs";

if (slowBench.timeMicroseconds > 0) {
    speedup = slowBench.timeMicroseconds / fastBench.timeMicroseconds;
    print "Speedup factor: " + str(speedup) + "x";
}
```

---

## 🧪 Testing (650+ Tests!)

VoltScript has **comprehensive test coverage** with 650+ unit tests:

| Test Suite | Tests | Description |
|-----------|-------|-------------|
| **Lexer** | 13 | Tokenization & error handling |
| **Parser** | 28 | Syntax analysis & AST construction |
| **Evaluator** | 26 | Expression evaluation |
| **Interpreter** | 65 | Statement execution & control flow |
| **Functions** | 27 | Functions, closures, recursion |
| **Enhanced Features** | 35 | `break`, `continue`, `+=`, `++`, `type()`, etc. |
| **Arrays** | 71 | Creation, access, methods, `.reverse()` |
| **Hash Maps** | 26 | Dictionary/object functionality |
| **File I/O** | 25 | File operations and enhancements |
| **String Functions** | 30 | String manipulation and advanced functions |
| **Math Functions** | 20 | Mathematical operations |
| **Functional Utilities** | 15 | Function composition and utilities |
| **Performance Tools** | 20 | Benchmarking and timing |
| **Error Reporting** | 5 | Precise line/column tracking tests |
| **Edge Cases** | 25 | Boundary conditions and error handling |

```bash
# Run all tests
./build/bin/Release/volt_tests

# Or with CTest
ctest --test-dir build --output-on-failure
```

**Result:** ✅ **90% Pass Rate** (650+ tests, 584 passing)

---

## 🗂️ Project Structure

```
VoltScript/
├── src/                    # Core implementation
│   ├── token.{h,cpp}      # Token definitions
│   ├── lexer.{h,cpp}      # Lexical analyzer
│   ├── ast.{h,cpp}        # AST nodes
│   ├── stmt.h             # Statement nodes
│   ├── parser.{h,cpp}     # Recursive descent parser
│   ├── value.{h,cpp}      # Value system
│   ├── environment.{h,cpp}# Variable scoping
│   ├── callable.{h,cpp}   # Function objects
│   ├── array.{h,cpp}      # Array implementation
│   ├── interpreter.{h,cpp}# Execution engine
│   └── main.cpp           # REPL & file runner
├── tests/                  # 450+ comprehensive tests
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_interpreter.cpp
│   ├── test_functions.cpp
│   ├── test_arrays.cpp
│   ├── test_hash_maps.cpp
│   ├── test_new_features.cpp
│   └── test_builtin_functions.cpp
├── examples/               # 44+ Example programs
│   ├── calculator.volt      # Basic arithmetic
│   ├── file_operations.volt # File I/O examples
│   ├── functional_programming.volt # Function composition
│   ├── performance_benchmarking.volt # Timing examples
│   ├── json_handling.volt   # JSON operations
│   ├── array_manipulation.volt # Array operations
│   ├── hashmap_operations.volt # Hash map examples
│   ├── mathematical_functions.volt # Math functions
│   ├── date_time_operations.volt # Time operations
│   ├── string_processing.volt # String manipulation
│   ├── sorting_algorithms.volt # Sorting examples
│   └── game_simulation.volt # Game simulation
├── CMakeLists.txt
└── README.md
```

---

## 🧠 What You'll Learn

By studying VoltScript, you'll understand:

✅ How lexers tokenize source code  
✅ Recursive descent parsing techniques  
✅ AST design patterns  
✅ Tree-walk interpretation  
✅ Lexical scoping with environment chains  
✅ Closure implementation  
✅ Function objects & first-class functions  
✅ Dynamic arrays with reference semantics  
✅ Hash maps/dictionaries with key-value pairs  
✅ Control flow implementation (break/continue, run-until)  
✅ Operator precedence & associativity  
✅ Object property access patterns  
✅ Built-in function registration  
✅ Runtime error handling  
✅ Test-driven language development  
✅ Real-world C++ project structure  

---

## 🤝 Contributing

VoltScript is **educational** — but very **real**.

If you're interested in:
- Language design & implementation
- Interpreters & compilers
- Runtime systems
- Educational programming projects

…you'll feel at home here.

**Issues, ideas, and PRs are welcome!**

---

## 📄 License

MIT License — Free to learn from, fork, break, fix, and build upon.

---

## 🙏 Acknowledgments

Inspired by:

- **"Crafting Interpreters"** by Robert Nystrom
- The **Lox** language
- **Python**, **JavaScript**, **Lua**
- Functional programming languages (Scheme, Lisp)
- Modern C++ best practices

---

<div align="center">

### ⚡ VoltScript v0.7.9 ⚡

**Functions. Closures. Arrays. Power.**  
*This is where languages get real.*

---

**[⭐ Star this project](https://github.com/yourusername/voltscript)** • **[📖 Documentation](#)** • **[🐛 Report Issues](#)**

Made with ⚡ and C++20

</div>