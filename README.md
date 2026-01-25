<div align="center">

# ⚡ VoltScript ⚡  
### *A programming language built from scratch in modern C++*

**Version:** `0.6.8`  
**Milestone:** Arrays & Enhanced Error Reporting
**Status:** 🟢 Production-Ready | 🚀 Feature-Rich | 🧠 Educational  

> *From tokens → trees → closures → arrays*  

[![Tests](https://img.shields.io/badge/tests-270%2F270-brightgreen)]()
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

## ✨ Features (v0.6.8)

### 🧩 Lexer — *Characters → Tokens*

- **Literals**: Numbers (`42`, `3.14`), Strings (`"hello"`), Booleans (`true`, `false`), `nil`
- **Identifiers**: `x`, `totalSum`, `_temp123`
- **Keywords**: `let`, `if`, `else`, `while`, `for`, `fn`, `return`, `print`, `break`, `continue`
- **Operators**:
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Logical: `&&`, `||`, `!`
  - Assignment: `=`
  - Compound: `+=`, `-=`, `*=`, `/=`
  - Increment/Decrement: `++`, `--` (prefix & postfix)
  - Ternary: `? :`
- **Punctuation**: `(`, `)`, `{`, `}`, `[`, `]`, `;`, `,`, `.`
- **Comments**: `// line comments`
- Precise error messages with line numbers

---

### 🌳 Parser — *Tokens → AST*

Hand-written **recursive descent parser**:

- **Expressions**:
  - Binary, unary, logical, grouping
  - Assignment & compound assignment (`+=`, `-=`, etc.)
  - Prefix/postfix increment/decrement
  - Ternary operator (`condition ? true_val : false_val`)
  - Function calls
  - Array literals (`[1, 2, 3]`)
  - Array indexing (`arr[i]`)
  - Member access (`arr.length`, `arr.push(x)`)

- **Statements**:
  - `print`, `let` declarations
  - Blocks `{ ... }`
  - `if` / `else`
  - `while`, `for`
  - `break`, `continue`
  - `fn` function declarations
  - `return`

- Correct operator precedence & associativity
- Helpful syntax error recovery
- Clean **Abstract Syntax Tree (AST)**

---

### 🧠 AST — *Program Structure Made Explicit*

- Separate **expression** and **statement** nodes
- Clear, inspectable tree structure
- Designed for interpretation now, compilation later
- Easy to debug and visualize

---

### ⚙️ Interpreter — *AST → Execution*

VoltScript runs **real programs** with:

- ✅ Arithmetic with correct precedence
- ✅ Variables with `let` + reassignment
- ✅ Block scoping & shadowing
- ✅ Control flow: `if`/`else`, `while`, `for`
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
- `len(x)` — Length of string or array
- `str(x)` — Convert to string
- `num(x)` — Convert to number
- `type(x)` — Get value type as string
- `toUpper(s)` — Convert string to uppercase
- `toLower(s)` — Convert string to lowercase

---

### 📦 Arrays — *Dynamic Collections*

Full array support with built-in methods:

```volt
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

```text
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
├── tests/                  # 261 comprehensive tests
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_evaluator.cpp
│   ├── test_interpreter.cpp
│   ├── test_functions.cpp
│   ├── test_enhanced_features.cpp
│   ├── test_arrays.cpp
│   └── test_error_reporting.cpp
├── examples/               # Example programs
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

```powershell
cmake -B build
cmake --build build --config Release
.\build\bin\Release\volt.exe
```

---

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/volt
```

---

### Run Tests

```bash
# Build and run all tests
cmake --build build --config Release
./build/bin/Release/volt_tests

# Or use CTest
ctest --output-on-failure --test-dir build
```

**Result:** 🟢 **270 / 270 tests passing (100%)**

---

## 💻 Using VoltScript

### Interactive REPL

```text
⚡ VoltScript v0.6.8 - Interactive REPL
Type 'exit' or 'quit' to leave, 'clear' to reset

> let arr = ;
> arr.push(4);
> print arr;


> fn sum(arr) {
    let total = 0;
    for (let i = 0; i < arr.length; i++) {
        total += arr[i];
    }
    return total;
}
> print sum();
60
```

---

### Run a File

```bash
volt script.volt
```

---

## 📝 Code Examples

### 🎯 Arrays & Loops

```volt
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

print sum();           // 15
print filterEvens(); // 
```

---

### 🎲 Nested Arrays (Matrices)

```volt
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
```

---

### 🔄 Closures with Arrays

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

```volt
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

## 🧪 Testing (270 Tests!)

VoltScript has **comprehensive test coverage** with 270 unit tests:

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

```bash
# Run all tests
./build/bin/Release/volt_tests

# Or with CTest
ctest --test-dir build --output-on-failure
```

**Result:** ✅ **100% Pass Rate** (270/270)

---

## 🚀 What's Next?

### Milestone 7: Objects & Hash Maps 🗺️
*Core data structure for real programs & Foundation for OOP*

- [ ] **Hash maps / Dictionaries**: `let person = {"name": "Alice", "age": 25};`
- [ ] **Property access**: `obj.name` and `obj["key"]` support
- [ ] **Dynamic addition**: `obj.newProp = value`
- [ ] **Nested objects**: `user.address.city`
- [ ] **Object methods**: Support for `this` keyword and methods
- [ ] **Built-ins**: `keys(obj)` and `values(obj)` functions

### Future Roadmap
- [ ] **String methods** — `.split()`, `.join()`, `.substring()`
- [ ] **More array methods** — `.map()`, `.filter()`, `.reduce()`
- [ ] **Classes & inheritance**
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
✅ Control flow implementation (break/continue)  
✅ Operator precedence & associativity  
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

### ⚡ VoltScript v0.6.8 ⚡

**Functions. Closures. Arrays. Power.**  
*This is where languages get real.*

---

**[⭐ Star this project](https://github.com/yourusername/voltscript)** • **[📖 Documentation](#)** • **[🐛 Report Issues](#)**

Made with ⚡ and C++20

</div>
```
