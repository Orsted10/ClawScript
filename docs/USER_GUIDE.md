# ClawScript User Guide (30 Minutes to Productive)

## 0–5 min: Install & Build
- Prereqs: CMake, C++20 compiler
- Build:
  - Windows: `cmake -S . -B build && cmake --build build --config Release`
  - Linux/macOS: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`
- Run REPL: `build/bin/Release/claw.exe` (Windows) or `./build/bin/claw`

## 5–10 min: First Script
```claw
fn greet(name) { print "Hello, " + name; }
greet("Claw");
```
- Save as `hello.claw` and run: `claw hello.claw`

## 10–15 min: Arrays & Maps
```claw
let a = [1,2,3]; a.push(4); print a.length;
let m = {}; m["key"] = "value"; print m["key"];
```

## 15–20 min: Functions & Classes
```claw
fn add(a,b){ return a+b; }
class Counter { fn inc(x){ return x+1; } }
let c = Counter(); print c.inc(41);
```

## 20–25 min: I/O & JSON
```claw
writeFile("data.txt", "hello");
let s = readFile("data.txt"); print s;
let o = jsonDecode("{\"a\":1}"); print o["a"];
```

## 25–30 min: Security & Logging
```claw
// Configure logging and allow output
writeFile(".voltsec", "log.path=app.log\nlog.hmac=abc123\nlog.meta.required=false\noutput=allow");
policyReload();

// Write log lines (metadata optional unless required by policy)
logWrite("startup", {"version":"2.0.0","pid":1234});
logWrite("heartbeat");

print readFile("app.log");
```

## 30–35 min: Tooling
- LSP: open workspace in VSCode, install extension in `vscode-extension/`
- Formatter: format on save or run `claw-fmt --write --root=.` 
- Profiler:
  - `claw --profile script.claw`
  - `CLAW_PROFILE=1 CLAW_PROFILE_OUT=profile.html claw script.claw`
