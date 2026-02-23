# ClawScript Performance Guide

## Targets
- fib(35): 30s → 2ms
- array(1M): 10s → 5ms
- mandelbrot: 60s → 10ms
- startup: 50ms → 2ms
- memory (1M obj): 500MB → 50MB

## Checklist
- Enable JIT aggressiveness: pass --jit=aggressive
- Use arrays and hash maps efficiently; pre-reserve when possible
- Avoid repeated global lookups; hoist to locals
- Prefer numeric loops and simple branches; avoid deep recursion
- Use fast math helpers: fibFast, arraySumFast
- Use class methods to reduce dynamic lookup cost
- Profile hotspots: --profile, adjust --profile-hz
- Build with PGO: set ENABLE_PGO=ON and PGO_PHASE=generate/use
- Use -O3 and LTO for AoT builds

## Profiling
- Enable profiling:
  - CLI: --profile[=file], --profile-hz=NUM
- Env: CLAW_PROFILE=1, CLAW_PROFILE_HZ, CLAW_PROFILE_OUT
- Outputs:
  - HTML flame graph, folded stacks, Speedscope JSON
- Control:
  - profilePause(), profileResume()

## JIT/AoT
- AoT: use --aot-output to emit object, then link with claw_runtime
- JIT: enable aggressive mode for hot loops and functions

## GC/Memory
- Avoid excessive temporary allocations
- Prefer preallocated arrays and reuse buffers
- Monitor heap flame for large growth sites: array.grow, hashmap.bucket.grow
