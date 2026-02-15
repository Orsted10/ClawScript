// Example: VM vs Interpreter loop performance (v0.9.0)
// Expected: Prints timing information and a speedup factor (numbers may vary).
//
// Prerequisites:
// - Build VoltScript in Release mode.
// - Run this with the standard volt binary (VM is used automatically).
//
// This example focuses on language-level benchmarking using the built-in
// benchmark() function and a simple loop workload.

fn tightLoop(n) {
    let i = 0;
    let acc = 0;
    while (i < n) {
        acc = acc + i;
        i = i + 1;
    }
    return acc;
}

print "=== VM Loop Benchmark (v0.9.0) ===";

let result = tightLoop(100000);
print "Result (sanity check): " + str(result);

let bench = benchmark(tightLoop, 100000);
print "Time (microseconds): " + str(bench.timeMicroseconds);

// Output will look like:
// === VM Loop Benchmark (v0.9.0) ===
// Result (sanity check): 4999950000
// Time (microseconds): 30000

