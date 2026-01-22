
<div align="center">

# ⚡ VoltScript ⚡  
### *A programming language built from scratch in modern C++*

**Version:** `0.6.0`  
**Milestone:** 6 — Functions & Closures  
**Status:** 🟢 Stable | 🚀 Powerful | 🧠 Educational  

> *From tokens → trees → closures*  

</div>

---

## 🧠 What is VoltScript?

VoltScript is a **real programming language**, written completely from scratch in **C++20**.

Not a transpiler.  
Not a wrapper.  
Not a toy.

This project exists to answer one question honestly:

> **How do programming languages actually work under the hood?**

So instead of hiding complexity, VoltScript **embraces it**—and builds every layer explicitly:
- lexical analysis
- parsing
- AST construction
- interpretation
- environments
- closures

If you’ve ever wanted to *truly* understand languages like Python, JavaScript, or Lua — this is the path.

---

## 🎯 Core philosophy

VoltScript is built with a few simple rules:

- 🧼 **Clarity over cleverness**
- 🧠 **Understanding over shortcuts**
- 🛠️ **Explicit behavior over magic**
- 📚 **Readable code over clever hacks**

Every subsystem is written to be:
- debuggable
- extendable
- educational
- real-world inspired

---

## ✨ What VoltScript can do (Milestones 1–6)

### 🧩 Lexer — *Characters → Tokens*
VoltScript understands:

- **Literals**
  - Numbers: `42`, `3.14`
  - Strings: `"hello world"`
  - Booleans: `true`, `false`
  - Nil: `nil`
- **Identifiers**
  - `x`, `total_sum`, `_temp123`
- **Keywords**
  - `let`, `if`, `else`, `while`, `for`, `fn`, `return`, `print`
- **Operators**
  - Arithmetic: `+ - * / %`
  - Comparison: `== != < <= > >=`
  - Logical: `&& || !`
  - Assignment: `=`
- **Punctuation**
  - `(` `)` `{` `}` `;` `,`
- Line comments: `// comment`
- Precise error messages with line numbers

---

### 🌳 Parser — *Tokens → AST*
A hand-written **recursive descent parser** that supports:

- Expressions:
  - unary, binary, logical, grouping, assignment
  - function calls
- Statements:
  - `print`
  - `let` declarations
  - blocks `{ ... }`
  - `if / else`
  - `while`
  - `for`
  - `fn`
  - `return`
- Correct operator precedence & associativity
- Helpful syntax error recovery
- Clean **Abstract Syntax Tree (AST)** output

---

### 🧠 AST — *Program Structure Made Explicit*
- Separate **expression** and **statement** nodes
- Clear, inspectable tree structure
- Designed for interpretation now, compilation later
- Easy to debug and visualize

---

### ⚙️ Interpreter — *AST → Execution*
VoltScript now runs **actual programs**:

- Arithmetic evaluation with correct precedence
- Variables with `let` + reassignment
- Block scoping & shadowing
- Control flow:
  - `if / else`
  - `while`
  - `for`
- `print` statement
- Comparison & logical operators
- **Short-circuit evaluation**
- String concatenation
- Runtime type checking
- Friendly runtime errors:
  - division by zero
  - undefined variables
  - type mismatches
  - wrong argument counts

---

### 🔁 Functions & Closures — *The Big One (Milestone 6)*

This is where VoltScript becomes **serious**.

- `fn` function declarations
- Parameters & return values
- Early `return`
- **First-class functions**
- **Closures** (functions capture surrounding scope)
- **Recursion**
- **Higher-order functions**
- Native (C++) functions callable from VoltScript

> At this point, VoltScript supports patterns used in  
> **Python, JavaScript, Lua, and functional languages**

---

## 🗂️ Project structure

```text
VoltScript/
├── src/           # Language implementation
├── tests/         # 157 unit tests (all passing)
├── examples/      # Real VoltScript programs
├── build/         # Generated output
└── README.md
````

(Yes, it’s clean on purpose.)

---

## 🛠️ Build & Run

### Requirements

* C++ compiler with **C++20 support**
* CMake ≥ 3.20

---

### Windows (MSVC)

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

## 💻 Using VoltScript

### Interactive REPL

```text
> fn add(a, b) { return a + b; }
> print add(5, 3);
8

> fn makeCounter() {
    let n = 0;
    fn inc() {
        n = n + 1;
        return n;
    }
    return inc;
}

> let counter = makeCounter();
> print counter();
1
> print counter();
2
```

---

### Example: Recursion

```volt
fn factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

print factorial(5); // 120
```

---

### Example: Closures

```volt
fn makeAdder(x) {
    fn add(y) {
        return x + y;
    }
    return add;
}

let add5 = makeAdder(5);
print add5(3); // 8
```

---

### Example: Higher-order functions

```volt
fn applyTwice(f, x) {
    return f(f(x));
}

fn double(n) { return n * 2; }

print applyTwice(double, 5); // 20
```

---

## 🧪 Testing (yes, we test properly)

VoltScript has **157 unit tests**.

✅ Lexer
✅ Parser
✅ Evaluator
✅ Interpreter
✅ Functions & closures

```bash
ctest --output-on-failure
```

**Result:**
🟢 **157 / 157 passing (100%)**

---

## 🚧 What’s next?

### Coming soon 🚀

* Arrays & lists
* Hash maps
* Standard library
* `break` / `continue`
* Classes & inheritance
* Bytecode compiler
* Virtual machine
* Garbage collection
* Module system

---

## 🧠 What you’ll learn here

* How lexers and parsers work
* Recursive descent parsing
* AST design
* Tree-walk interpretation
* Lexical scoping & closures
* Function objects
* Call stacks & environments
* Control flow internals
* Error handling strategies
* Test-driven development in C++
* Writing *real* language code

---

## 🤝 Contributing

VoltScript is educational — but **very real**.

If you like:

* language internals
* interpreters
* compilers
* runtime systems

…you’ll feel at home here.

Issues, ideas, and PRs are welcome.

---

## 📄 License

Free to learn from, fork, break, fix, and build upon.

---

## 🙏 Acknowledgments

Inspired by:

* *Crafting Interpreters* — Robert Nystrom
* The Lox language
* Functional languages (Scheme, JS)
* Modern C++ best practices

---

<div align="center">

### ⚡ VoltScript v0.6.0

**Functions. Closures. Power.**
*This is where languages get real.*

</div>
```
