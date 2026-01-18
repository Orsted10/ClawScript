````markdown
# VoltScript ⚡  
*A modern programming language built from scratch in C++20*

**Current version:** `0.3.0`  
**Status:** Milestone 3 complete — Parser & AST finished

---

## Why VoltScript exists

VoltScript is an **educational, from-scratch programming language implementation** written in
**modern C++ (C++20)**.

The goal is not to clone an existing language, but to **understand how real programming languages are
built internally** — step by step — while keeping the design clean, readable, and extendable.

This project focuses on:
- correctness over shortcuts  
- clarity over cleverness  
- learning how real compilers and interpreters work  

VoltScript is being developed in **milestones**, each adding a core piece of a real language pipeline.

---

## What’s implemented so far (Milestones 1–3)

### ✅ Lexer (Tokenizer)
- Numbers (`42`, `3.14`)
- Strings (`"hello world"`)
- Booleans (`true`, `false`)
- `nil`
- Identifiers (`x`, `myVar`, `_test123`)
- Keywords:  
  `let`, `if`, `else`, `while`, `for`, `fn`, `return`
- Operators:
  - Arithmetic: `+ - * / %`
  - Comparison: `== != < <= > >=`
  - Logical: `&& || !`
  - Assignment: `=`
- Punctuation: `() {} ; ,`
- Line comments (`//`)
- Precise error reporting with line numbers

---

### ✅ Parser (Recursive Descent)
- Fully recursive descent parser
- Correct operator precedence and associativity
- Unary, binary, grouping, and assignment expressions
- Function calls with arguments
- Meaningful syntax errors and recovery
- Outputs a **clean Abstract Syntax Tree (AST)**

---

### ✅ Abstract Syntax Tree (AST)
- Strongly typed AST node hierarchy
- Represents program structure explicitly
- Designed to be consumed by an interpreter or VM later
- Easy to debug and visualize

---

## Project layout

```
VoltScript/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── src/
│   ├── token.h / token.cpp     # Token definitions
│   ├── lexer.h / lexer.cpp     # Lexical analyzer
│   ├── ast.h / ast.cpp         # AST node definitions
│   ├── parser.h / parser.cpp   # Recursive descent parser
│   └── main.cpp                # Entry point
├── tests/
│   ├── test_lexer.cpp          # Lexer unit tests
│   └── test_parser.cpp         # Parser unit tests
└── build/                      # Build output (generated)
    └── bin/
        ├── volt                # Language executable
        └── volt_tests          # Test suite
````

---

## Build requirements

* **C++ compiler**
  GCC 10+, Clang 12+, or MSVC 2019+ (C++20 required)
* **CMake** ≥ 3.20
* **Google Test**
  Automatically fetched via CMake (no manual install needed)

---

## Building VoltScript

### Windows (Visual Studio / MSVC)

```powershell
cmake -B build
cmake --build build --config Release
```

Run:

```powershell
.\build\bin\Release\volt.exe "1 + 2 * 3"
```

---

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run:

```bash
./build/bin/volt "1 + 2 * 3"
```

---

## Using VoltScript (current stage)

VoltScript currently **parses expressions and prints their AST**.

### Simple expressions

```bash
./volt "1 + 2 * 3"
```

AST output:

```
(+ 1.000000 (* 2.000000 3.000000))
```

Meaning:

```
1 + (2 * 3)
```

---

### Parentheses (grouping)

```bash
./volt "(1 + 2) * 3"
```

AST:

```
(* (group (+ 1.000000 2.000000)) 3.000000)
```

---

### Variables and assignment

```bash
./volt "x = (a + b) * c"
```

AST:

```
(= x (* (group (+ a b)) c))
```

---

### Logical and comparison expressions

```bash
./volt "x > 5 && y < 10"
```

AST:

```
(&& (> x 5.000000) (< y 10.000000))
```

---

### Function calls

```bash
./volt "max(min(a, b), c)"
```

AST:

```
(call max (call min a b) c)
```

---

## Operator precedence (implemented)

| Level | Operators                       | Associativity |   |      |
| ----: | ------------------------------- | ------------- | - | ---- |
|     1 | Literals, identifiers, grouping | —             |   |      |
|     2 | Unary `! -`                     | Right         |   |      |
|     3 | `* / %`                         | Left          |   |      |
|     4 | `+ -`                           | Left          |   |      |
|     5 | `< > <= >=`                     | Left          |   |      |
|     6 | `== !=`                         | Left          |   |      |
|     7 | `&&`                            | Left          |   |      |
|     8 | `                               | Left          |   |      |
|     9 | `=`                             | Right         |   |      |

---

## Testing

VoltScript uses **Google Test** and currently has **21 passing unit tests**.

Run all tests:

```bash
cd build
ctest --output-on-failure
```

Run specific tests:

```bash
./bin/volt_tests --gtest_filter=Parser.*
```

---

## Error handling examples

### Lexer errors

```bash
./volt '"hello'
```

```
Error at line 1: Unterminated string
```

---

### Parser errors

```bash
./volt "(1 + 2"
```

```
[Line 1] Error: Expected ')' after expression
```

---

## Roadmap

### Completed

* [x] Tokenizer (Lexer)
* [x] Keywords and operators
* [x] Recursive descent parser
* [x] AST generation
* [x] Operator precedence
* [x] Error reporting

### Coming next

* [ ] Tree-walk interpreter
* [ ] Runtime environment
* [ ] Variables and scopes
* [ ] Control flow (`if`, `while`, `for`)
* [ ] Functions and closures
* [ ] Standard library
* [ ] Bytecode compiler
* [ ] Virtual machine

---

## Architecture overview

```text
Input source code
        ↓
      Lexer
        ↓
     Tokens
        ↓
      Parser
        ↓
       AST
        ↓
  Interpreter (next milestone)
```

---

## Contributing

This is an **educational project**, but real and serious.

If you want to:

* explore language internals
* improve error handling
* add new syntax
* help with the interpreter

Feel free to open issues or submit PRs.

---

## License

Free to learn from, fork, and build upon.

---

**VoltScript**
Built with curiosity, frustration, and a lot of C++ ☕
`v0.3.0 — Milestone 3 complete`
