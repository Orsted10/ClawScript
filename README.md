<div align="center">

# ⚡ VoltScript ⚡  
### *A programming language built from scratch in modern C++*

**Version:** `0.7.0`  
**Milestone:** Objects & Hash Maps 🗺️
**Status:** 🟢 Production-Ready | 🚀 Feature-Rich | 🧠 Educational  

> *From tokens → trees → closures → arrays → objects*  

[![Tests](https://img.shields.io/badge/tests-320%2F320-brightgreen)]()
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

</div>

---

## 🧠 What is VoltScript?

VoltScript is a **fully-functional programming language**, written completely from scratch in **C++20**.

Not a transpiler.  
Not a wrapper.  
Not a toy.

This project exists to answer one question honestly:

> **How do programming languages actually work under the hood?**

So instead of hiding complexity, VoltScript **embraces it**—and builds every layer explicitly:
- ⚡ Lexical analysis
- 🌳 Parsing with recursive descent
- 🧠 AST construction
- ⚙️ Tree-walk interpretation
- 📍 Precise error reporting (Line/Col tracking)
- 🔐 Lexical scoping & environments
- 🔁 First-class functions & closures
- 📦 Dynamic arrays with methods
- 🎯 Compound operators & control flow

If you've ever wanted to *truly* understand languages like **Python**, **JavaScript**, or **Lua** — this is your path.

---

## 🎯 Core Philosophy

VoltScript is built with a few simple rules:

- 🧼 **Clarity over cleverness**
- 🧠 **Understanding over shortcuts**
- 🛠️ **Explicit behavior over magic**
- 📚 **Readable code over clever hacks**

Every subsystem is written to be:
- Debuggable
- Extendable
- Educational
- Real-world inspired

---

## ✨ Features (v0.7.2)

### 🧩 Lexer — *Characters → Tokens*

- **Literals**: Numbers (`42`, `3.14`), Strings (`"hello"`), Booleans (`true`, `false`), `nil`
- **Identifiers**: `x`, `totalSum`, `_temp123`
- **Keywords**: `let`, `if`, `else`, `while`, `for`, `run`, `until`, `fn`, `return`, `print`, `break`, `continue`
- **Operators**:
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Logical: `&&`, `||`, `!`
  - Assignment: `=`
  - Compound: `+=`, `-=`, `*=`, `/=`
  - Increment/Decrement: `++`, `--` (prefix & postfix)
  - Ternary: `? :`
  - Indexing: `[]` (for arrays and hash maps)
- **Punctuation**: `(`, `)`, `{`, `}`, `[`, `]`, `;`, `,`, `.`
- **Comments**: `// line comments`
- **Literals**: Numbers (`42`, `3.14`), Strings (`"hello"`), Booleans (`true`, `false`), `nil`, **Hash Maps** (`{"key": "value"}`)

---

### 🧠 AST — *Program Structure Made Explicit*

- Separate **expression** and **statement** nodes
- **Hash Map literals** support: `{"key": value, "another": 42}`
- Clear, inspectable tree structure
- Designed for interpretation now, compilation later
- Easy to debug and visualize

---

### ⚙️ Interpreter — *AST → Execution*

VoltScript runs **real programs** with:

- ✅ Arithmetic with correct precedence
- ✅ Variables with `let` + reassignment
- ✅ Block scoping & shadowing
- ✅ Control flow: `if`/`else`, `while`, `for`, `run-until`
- ✅ **Loop control**: `break` and `continue`
- ✅ `print` statement
- ✅ Comparison & logical operators
- ✅ **Short-circuit evaluation**
- ✅ String concatenation & coercion
- ✅ **Compound assignment**: `x += 5`, `s *= 2`
- ✅ **Increment/Decrement**: `++i`, `i--`, etc.
- ✅ **Ternary operator**: `max = (a > b) ? a : b;`
- ✅ **Precise error reporting**: Errors point to exact source tokens
- ✅ Runtime type checking with friendly errors
- ✅ **Hash Map support**: `{ "key": "value", "nested": { "prop": 42 } }`
- ✅ **Hash Map operations**: Access with `map["key"]`, assignment with `map["key"] = value`
- ✅ Built-in functions: `keys(map)`, `values(map)`, `has(map, key)`, `remove(map, key)`
- ✅ Hash Map member access: `map.size`, `map.keys()`, `map.values()`, `map.has(key)`, `map.remove(key)`

---

### 🔁 Functions & Closures

VoltScript supports **advanced function features**:

- ✅ `fn` declarations
- ✅ Parameters & return values
- ✅ Early `return`
- ✅ **First-class functions** (functions as values)
- ✅ **Closures** (capture surrounding scope)
- ✅ **Recursion**
- ✅ **Higher-order functions**
- ✅ **Native C++ functions** callable from VoltScript

Built-in functions:
- `clock()` — Returns current timestamp
- `len(x)` — Length of string, array, or hash map
- `str(x)` — Convert to string
- `num(x)` — Convert to number
- `type(x)` — Get value type as string
- `toUpper(s)` — Convert string to uppercase
- `toLower(s)` — Convert string to lowercase
- `upper(s)` — Convert string to uppercase (alias)
- `lower(s)` — Convert string to lowercase (alias)
- `substr(s, start, length)` — Extract substring
- `indexOf(s, substr)` — Find substring position
- `trim(s)` — Remove whitespace from both ends
- `split(s, delimiter)` — Split string into array
- `replace(s, search, replacement)` — Replace all occurrences
- `startsWith(s, prefix)` — Check if string starts with prefix
- `endsWith(s, suffix)` — Check if string ends with suffix
- `keys(map)` — Get hash map keys as array
- `values(map)` — Get hash map values as array
- `has(map, key)` — Check if key exists in hash map
- `remove(map, key)` — Remove key-value pair from hash map
- `abs(x)` — Absolute value
- `sqrt(x)` — Square root
- `pow(base, exponent)` — Power function
- `min(a, b)` — Minimum of two values
- `max(a, b)` — Maximum of two values
- `round(x)` — Round to nearest integer
- `floor(x)` — Round down to integer
- `ceil(x)` — Round up to integer
- `random()` — Random number between 0 and 1

---

### 📦 Arrays — *Dynamic Collections*

Full array support with built-in methods:

``volt
let numbers = ;
let mixed = [42, "hello", true, nil];
let nested = [, ];

// Access
print numbers;          // 1
print nested;        // 3

// Modify
numbers = 99;
print numbers;             

// Properties & Methods
print numbers.length;      // 5
numbers.push(6);           // Add element
let last = numbers.pop();  // Remove and return last

// Iterate
for (let i = 0; i < numbers.length; i++) {
    print numbers[i];
}
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
- ✅ Trailing commas: `[1, 2, 3,]`
- ✅ Bounds checking with helpful errors

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
├── tests/                  # 345 comprehensive tests
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_evaluator.cpp
│   ├── test_interpreter.cpp
│   ├── test_functions.cpp
│   ├── test_enhanced_features.cpp
│   ├── test_arrays.cpp
│   └── test_error_reporting.cpp
├── examples/               # Example programs
│   ├── calculator.volt      # Basic arithmetic
│   ├── counter_closure.volt # Closure demonstration
│   ├── fibonacci.volt       # Fibonacci sequence
│   ├── hash_maps.volt       # Hash map usage
│   ├── enhanced_features.volt # v0.7.1 features
│   └── v072_enhancements.volt # New v0.7.2 features
├── CMakeLists.txt
└── README.md
```

---

## 🛠️ Build & Run

### Requirements

* C++ compiler with **C++20 support** (MSVC 19.28+, GCC 10+, Clang 11+)
* CMake ≥ 3.14
* GoogleTest (automatically fetched)

---

### Windows (MSVC / Visual Studio)

```
cmake -B build
cmake --build build --config Release
.\build\bin\Release\volt.exe
```

---

### Linux / macOS

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/volt
```

---

### Run Tests

```
# Build and run all tests
cmake --build build --config Release
./build/bin/Release/volt_tests

# Or use CTest
ctest --output-on-failure --test-dir build
```

**Result:** 🟢 **294 / 294 tests passing (100%)**

---

## 💻 Using VoltScript

### Interactive REPL

```
⚡ VoltScript v0.7.0 - Interactive REPL
Type 'exit' or 'quit' to leave, 'clear' to reset

> let person = {"name": "Alice", "age": 25};
> print person["name"];
Alice
> person.email = "alice@example.com";  // TODO: Object properties
> let nums = [1, 2, 3];
> print nums.length;
3
```

---

### Run a File

```bash
volt script.volt
```

---

## 📝 Code Examples

### 🎯 Arrays & Loops

```
// Build Fibonacci sequence
let fib = ;
for (let i = 0; i < 8; i++) {
    let next = fib[fib.length - 1] + fib[fib.length - 2];
    fib.push(next);
}
print fib;  // 
```

---

### 🔁 Array Functions

```
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

print sum();           // 15
print filterEvens(); // 
```

---

### 🎲 Nested Arrays (Matrices)

```
let matrix = [
    ,
    ,
    
];

// Access elements
print matrix;  // 6

// Iterate
for (let i = 0; i < matrix.length; i++) {
    for (let j = 0; j < matrix[i].length; j++) {
        print matrix[i][j];
    }
}
```

---

### 🚦 Enhanced Control Flow

```
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

---

### 🔄 Closures with Arrays

```
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
let push = stack;
let pop = stack;
let size = stack;

push(10);
push(20);
print pop();   // 20
print size();  // 1
```

---

### 🎯 Practical Example: Find Maximum

```
fn max(arr) {
    if (arr.length == 0) return nil;
    
    let maxVal = arr;
    for (let i = 1; i < arr.length; i++) {
        if (arr[i] > maxVal) {
            maxVal = arr[i];
        }
    }
    return maxVal;
}

print max();  // 9
```

---

## 🧪 Testing (320 Tests!)

VoltScript has **comprehensive test coverage** with 320 unit tests:

| Test Suite | Tests | Description |
|-----------|-------|-------------|
| **Lexer** | 13 | Tokenization & error handling |
| **Parser** | 28 | Syntax analysis & AST construction |
| **Evaluator** | 26 | Expression evaluation |
| **Interpreter** | 65 | Statement execution & control flow |
| **Functions** | 27 | Functions, closures, recursion |
| **Enhanced Features** | 35 | `break`, `continue`, `+=`, `++`, `type()`, etc. |
| **Arrays** | 71 | Creation, access, methods, `.reverse()` |
| **Error Reporting** | 5 | Precise line/column tracking tests |
| **Run-Until Loops** | 24 | Do-until style loops with break/continue |
| **Hash Maps** | 26 | Dictionary/object functionality, keys/values, access |

```
# Run all tests
./build/bin/Release/volt_tests

# Or with CTest
ctest --test-dir build --output-on-failure
```

**Result:** ✅ **100% Pass Rate** (320/320)

---

## 🚀 What's Next?

### Milestone 8: Object-Oriented Programming 🏗️
*Real encapsulation, inheritance, and polymorphism*

- [ ] **Class declarations**: `class Person { constructor(name) { this.name = name; } }`
- [ ] **Instance methods**: `obj.method()` with proper `this` binding
- [ ] **Inheritance**: `class Employee extends Person {}`
- [ ] **Encapsulation**: Private/public members
- [ ] **Method resolution**: Super calls, overloading

### Future Roadmap
- [ ] **String methods** — `.split()`, `.join()`, `.substring()`
- [ ] **More array methods** — `.map()`, `.filter()`, `.reduce()`
- [ ] **Exception handling** — `try`/`catch`
- [ ] **Module system** — `import`/`export`
- [ ] **Standard library**
- [ ] **Bytecode compiler + VM** (for 10-100x speed improvement)
- [ ] **Garbage collection** (currently uses shared_ptr)
- [ ] **Debugger integration**

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

### ⚡ VoltScript v0.7.2 ⚡

**Functions. Closures. Arrays. Power.**  
*This is where languages get real.*

---

**[⭐ Star this project](https://github.com/yourusername/voltscript)** • **[📖 Documentation](#)** • **[🐛 Report Issues](#)**

Made with ⚡ and C++20

</div>