````markdown
# VoltScript ⚡  
*A programming language built from scratch in modern C++*

**Version:** `0.4.0`  
**Status:** Milestone 4 complete — Interpreter working 🎉

---

## About VoltScript

VoltScript is an **educational programming language implementation** written entirely from scratch in **C++20**.

This project exists for one reason:  
to **deeply understand how real programming languages work internally** — not by reading theory alone, but by *building every layer yourself*.

VoltScript is **not** a clone of any existing language.  
Instead, it is a clean, minimal, well-structured language designed to teach:

- how lexers tokenize source code  
- how parsers build syntax trees  
- how interpreters execute programs  
- how runtime environments store and resolve values  

The project grows **milestone by milestone**, each one adding a real subsystem used in production languages.

---

## Current capabilities (Milestones 1–4)

### ✅ Lexical Analysis (Lexer)
VoltScript can tokenize:
- Numbers (`42`, `3.14`, `-5`)
- Strings (`"hello world"`)
- Booleans (`true`, `false`)
- `nil`
- Identifiers (`x`, `total_sum`, `_value123`)
- Keywords:  
  `let`, `if`, `else`, `while`, `for`, `fn`, `return`
- Operators:
  - Arithmetic: `+ - * / %`
  - Comparison: `== != < <= > >=`
  - Logical: `&& || !`
  - Assignment: `=`
- Punctuation: `() {} ; ,`
- Line comments (`// comment`)
- Accurate error reporting with line numbers

---

### ✅ Syntax Analysis (Parser)
- Hand-written **recursive descent parser**
- Correct operator precedence and associativity
- Unary, binary, grouping, and assignment expressions
- Function call syntax (evaluation coming later)
- Helpful syntax errors with recovery
- Produces a **well-structured Abstract Syntax Tree (AST)**

---

### ✅ Abstract Syntax Tree (AST)
- Strongly typed AST node hierarchy
- Explicit representation of program structure
- Easy to debug and visualize
- Designed to support interpretation now and compilation later

---

### ✅ Interpreter (Tree-Walk Evaluator) — *Milestone 4*
VoltScript can now **execute code**, not just parse it.

Supported at runtime:
- Arithmetic evaluation with precedence
- Comparison operators returning booleans
- Logical operators with **short-circuit evaluation**
- Variables with assignment and lookup
- String concatenation
- Runtime type checking
- Runtime error handling:
  - Division by zero
  - Undefined variables
  - Type mismatches
- Multiple expressions using `;`
- Persistent runtime environment

---

## Project structure

```text
VoltScript/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── token.h / token.cpp          # Token definitions
│   ├── lexer.h / lexer.cpp          # Lexical analyzer
│   ├── ast.h / ast.cpp              # AST node definitions
│   ├── parser.h / parser.cpp        # Recursive descent parser
│   ├── value.h / value.cpp          # Runtime value system
│   ├── environment.h / environment.cpp
│   ├── evaluator.h / evaluator.cpp  # Tree-walk interpreter
│   └── main.cpp                     # Entry point
├── tests/
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   └── test_evaluator.cpp
└── build/                           # Generated build output
    └── bin/
        ├── volt
        └── volt_tests
````

---

## Building VoltScript

### Requirements

* **C++ compiler**: GCC 10+, Clang 12+, or MSVC 2019+
* **CMake** ≥ 3.20
* **Google Test** (fetched automatically)

---

### Windows (MSVC / Visual Studio)

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
./build/bin/volt "1 + 2 * 3"
```

---

## Using VoltScript

VoltScript now **executes expressions and prints results**.

### Arithmetic & precedence

```bash
./volt "1 + 2 * 3"
```

Output:

```
7
```

```bash
./volt "(1 + 2) * 3"
```

Output:

```
9
```

---

### Variables

```bash
./volt "x = 10; x * 2"
```

Output:

```
20
```

```bash
./volt "result = (10 + 5) * 2; result - 5"
```

Output:

```
25
```

---

### Logic & comparisons

```bash
./volt "(5 > 3) && (10 < 20)"
```

Output:

```
true
```

Short-circuit evaluation:

```bash
./volt "false && (10 / 0)"
```

Output:

```
false
```

(No runtime error — evaluation stops early.)

---

### Strings

```bash
./volt '"hello" + " world"'
```

Output:

```
hello world
```

---

## Operator precedence

| Level | Operators          | Associativity |   |      |
| ----: | ------------------ | ------------- | - | ---- |
|     1 | Literals, grouping | —             |   |      |
|     2 | Unary `! -`        | Right         |   |      |
|     3 | `* / %`            | Left          |   |      |
|     4 | `+ -`              | Left          |   |      |
|     5 | `< > <= >=`        | Left          |   |      |
|     6 | `== !=`            | Left          |   |      |
|     7 | `&&`               | Left          |   |      |
|     8 | `=`                | Right         |   |      |

---

## Testing

VoltScript uses **Google Test** and currently has **50 passing unit tests**.

```bash
cd build
ctest --output-on-failure
```

Test breakdown:

* Lexer: 8 tests
* Parser: 13 tests
* Evaluator: 29 tests

---

## Error handling examples

```bash
./volt "10 / 0"
```

```
Runtime Error [Line 1]: Division by zero
```

```bash
./volt "x + 5"
```

```
Runtime Error [Line 1]: Undefined variable: x
```

```bash
./volt '5 + "hello"'
```

```
Runtime Error [Line 1]: Operands must be two numbers or two strings
```

---

## How VoltScript works (high level)

```text
Source code
   ↓
 Lexer        → Tokens
   ↓
 Parser       → AST
   ↓
 Evaluator    → Runtime result
```

Each step is implemented explicitly and kept readable for learning.

---

## Roadmap

### Completed

* Lexer
* Parser
* AST
* Interpreter
* Runtime environment
* Type checking
* Error handling

### Coming next

* Statements (`if`, `while`, `for`)
* Functions & closures
* Standard library
* Classes & objects
* Bytecode compiler
* Virtual machine (VM)

---

## Contributing

VoltScript is an **educational but serious project**.

If you’re interested in:

* language internals
* interpreter design
* runtime systems
* compilers & VMs

feel free to open issues, suggest improvements, or submit PRs.

---

## License

MIT License — free to learn from, fork, and build upon.

---

**VoltScript ⚡**
From tokens → trees → execution
`v0.4.0 — Interpreter milestone complete`
