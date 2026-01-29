// Example: Performance Benchmarking in VoltScript
print("=== Performance Benchmarking Demo ===");

// Define functions to benchmark
slowFunction = fun(n) {
    // Simulate some computation
    result = 0;
    for (let i = 0; i < n; i = i + 1) {
        result = result + i * i;
    }
    return result;
};

fastFunction = fun(n) {
    // Fast computation
    return n * (n + 1) / 2;
};

// Benchmark the slow function
print("Benchmarking slow function...");
benchSlow = benchmark(slowFunction, 1000);
print("Result: " + str(benchSlow.result));
print("Time (ms): " + str(benchSlow.timeMilliseconds));
print("Time (μs): " + str(benchSlow.timeMicroseconds));

// Benchmark the fast function
print("\nBenchmarking fast function...");
benchFast = benchmark(fastFunction, 1000);
print("Result: " + str(benchFast.result));
print("Time (ms): " + str(benchFast.timeMilliseconds));
print("Time (μs): " + str(benchFast.timeMicroseconds));

// Compare performance
speedup = benchSlow.timeMicroseconds / benchFast.timeMicroseconds;
print("\nSpeedup factor: approximately " + str(speedup) + "x");

// Timing different sizes
sizes = [100, 500, 1000];
print("\nTiming for different input sizes:");
for (let i = 0; i < sizes.length; i = i + 1) {
    size = sizes[i];
    timing = benchmark(slowFunction, size);
    print("Size " + str(size) + ": " + str(timing.timeMilliseconds) + " ms");
}

// Using sleep for timing demonstrations
print("\nDemonstrating sleep function:");
print("Before sleep...");
sleep(1000);  // Sleep for 1 second
print("After sleep (1 second delay)");

// Practical example: Benchmark different algorithms
fibRecursive = fun(n) {
    if (n <= 1) return n;
    return fibRecursive(n - 1) + fibRecursive(n - 2);
};

fibIterative = fun(n) {
    if (n <= 1) return n;
    a = 0;
    b = 1;
    for (let i = 2; i <= n; i = i + 1) {
        temp = a + b;
        a = b;
        b = temp;
    }
    return b;
};

// Be careful with recursive Fibonacci - use small n
print("\nComparing Fibonacci implementations (n=20):");
fibRecBench = benchmark(fibRecursive, 20);
print("Recursive Fibonacci: " + str(fibRecBench.timeMicroseconds) + " μs");

fibIterBench = benchmark(fibIterative, 20);
print("Iterative Fibonacci: " + str(fibIterBench.timeMicroseconds) + " μs");

// Performance testing utility function
performanceTest = fun(func, inputs, desc) {
    print("\nPerformance test: " + desc);
    totalTime = 0;
    for (let i = 0; i < inputs.length; i = i + 1) {
        bench = benchmark(func, inputs[i]);
        totalTime = totalTime + bench.timeMicroseconds;
        print("  Input " + str(inputs[i]) + ": " + str(bench.timeMicroseconds) + " μs");
    }
    avgTime = totalTime / inputs.length;
    print("  Average time: " + str(avgTime) + " μs");
};

// Run a performance test
inputs = [10, 20, 30];
performanceTest(fastFunction, inputs, "Fast summation function");