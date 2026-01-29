// Example: Date and Time Operations in VoltScript
print("=== Date and Time Operations Demo ===");

// Get current timestamp
currentTimestamp = now();
print("Current timestamp (milliseconds since epoch): " + str(currentTimestamp));

// Sleep example
print("Sleeping for 1 second...");
sleep(1000);  // Sleep for 1 second
print("Awake!");

// Timing operations
startTime = now();
print("Starting timed operation...");
// Simulate some work
counter = 0;
for (let i = 0; i < 100000; i = i + 1) {
    counter = counter + 1;
}
endTime = now();
elapsedTime = endTime - startTime;
print("Operation took " + str(elapsedTime) + " milliseconds");

// Benchmark different operations
operation1 = fun() {
    sum = 0;
    for (let i = 0; i < 10000; i = i + 1) {
        sum = sum + i;
    }
    return sum;
};

operation2 = fun() {
    product = 1;
    for (let i = 1; i <= 100; i = i + 1) {
        product = product * 1.001;
    }
    return product;
};

print("\nBenchmarking operations:");
bench1 = benchmark(operation1);
bench2 = benchmark(operation2);

print("Operation 1 (sum calculation):");
print("  Result: " + str(bench1.result));
print("  Time: " + str(bench1.timeMicroseconds) + " microseconds");

print("Operation 2 (product calculation):");
print("  Result: " + str(bench2.result));
print("  Time: " + str(bench2.timeMicroseconds) + " microseconds");

// Timing different array sizes
calculateSum = fun(n) {
    sum = 0;
    for (let i = 0; i < n; i = i + 1) {
        sum = sum + i;
    }
    return sum;
};

print("\nTiming with different input sizes:");
sizes = [1000, 5000, 10000];
for (let i = 0; i < sizes.length; i = i + 1) {
    size = sizes[i];
    timing = benchmark(calculateSum, size);
    print("  Size " + str(size) + ": " + str(timing.timeMicroseconds) + " μs (result: " + str(timing.result) + ")");
}

// Sleep for different durations
durations = [100, 200, 300];  // milliseconds
print("\nSleeping for different durations:");
for (let i = 0; i < durations.length; i = i + 1) {
    duration = durations[i];
    print("  Sleeping for " + str(duration) + "ms...");
    sleep(duration);
    print("    Done!");
}

// Performance comparison
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

print("\nPerformance comparison (n=100):");
slowBench = benchmark(slowFunction, 100);
fastBench = benchmark(fastFunction, 100);

print("Slow O(n²) function: " + str(slowBench.timeMicroseconds) + " μs");
print("Fast O(1) function: " + str(fastBench.timeMicroseconds) + " μs");

if (slowBench.timeMicroseconds > 0) {
    speedup = slowBench.timeMicroseconds / fastBench.timeMicroseconds;
    print("Speedup factor: approximately " + str(speedup) + "x");
}

// Timing with warm-up to reduce variance
warmUpFunction = fun() {
    // Perform operation multiple times to warm up
    for (let i = 0; i < 100; i = i + 1) {
        calculateSum(1000);
    }
};

print("\nWarm-up run...");
warmUpResult = benchmark(warmUpFunction);
print("Warm-up completed in " + str(warmUpResult.timeMicroseconds) + " μs");

print("Timed run after warm-up:");
timedResult = benchmark(calculateSum, 1000);
print("Main run: " + str(timedResult.timeMicroseconds) + " μs");

// Practical example: Animation frame timing
simulateAnimation = fun(frames) {
    frameTimes = [];
    for (let i = 0; i < frames; i = i + 1) {
        frameStart = now();
        // Simulate frame processing
        dummy = 0;
        for (let j = 0; j < 1000; j = j + 1) {
            dummy = dummy + 1;
        }
        frameEnd = now();
        frameTime = frameEnd - frameStart;
        frameTimes.push(frameTime);
        
        // Target 60 FPS = ~16.67ms per frame
        targetFrameTime = 16.67;
        if (frameTime < targetFrameTime) {
            sleep(targetFrameTime - frameTime);
        }
    }
    return frameTimes;
};

print("\nSimulating animation timing (5 frames):");
animationFrameTimes = simulateAnimation(5);
totalAnimationTime = 0;
for (let i = 0; i < animationFrameTimes.length; i = i + 1) {
    totalAnimationTime = totalAnimationTime + animationFrameTimes[i];
}
avgFrameTime = totalAnimationTime / animationFrameTimes.length;
print("Average frame time: " + str(avgFrameTime) + " ms");
print("Total animation time: " + str(totalAnimationTime) + " ms");

// Micro-benchmarking utility
microBenchmark = fun(func, iterations) {
    start = now();
    for (let i = 0; i < iterations; i = i + 1) {
        func();
    }
    end = now();
    totalTime = end - start;
    avgTime = totalTime / iterations;
    return {
        "totalTimeMs": totalTime,
        "avgTimePerCallMs": avgTime,
        "iterations": iterations
    };
};

simpleFunc = fun() { return 42; };
mbResult = microBenchmark(simpleFunc, 10000);

print("\nMicro-benchmarking:");
print("  Function calls: " + str(mbResult.iterations));
print("  Total time: " + str(mbResult.totalTimeMs) + " ms");
print("  Average per call: " + str(mbResult.avgTimePerCallMs) + " ms");