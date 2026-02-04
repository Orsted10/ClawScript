// Performance benchmarking and timing demonstration
print "=== Performance Benchmarking ===";

// Measure simple operations
print "=== Basic Timing ===";
let startTime = now();
let endTime = now();
print "Time measurement overhead: " + str(endTime - startTime) + " ms";

// Benchmark different loop types
print "\n=== Loop Performance Comparison ===";

// For loop benchmark
let startTime1 = now();
let sum1 = 0;
for (let i = 0; i < 10000; i = i + 1) {
    sum1 = sum1 + i;
}
let endTime1 = now();
print "For loop (10000 iterations): " + str(endTime1 - startTime1) + " ms";
print "Sum: " + str(sum1);

// While loop benchmark
let startTime2 = now();
let sum2 = 0;
let counter = 0;
while (counter < 10000) {
    sum2 = sum2 + counter;
    counter = counter + 1;
}
let endTime2 = now();
print "While loop (10000 iterations): " + str(endTime2 - startTime2) + " ms";
print "Sum: " + str(sum2);

// Array operations benchmark
print "\n=== Array Operations Performance ===";

// Create large array
let startTime3 = now();
let largeArray = [];
for (let i = 0; i < 5000; i = i + 1) {
    largeArray.push(i);
}
let endTime3 = now();
print "Array creation (5000 elements): " + str(endTime3 - startTime3) + " ms";

// Array iteration
let startTime4 = now();
let arraySum = 0;
for (let i = 0; i < largeArray.length; i = i + 1) {
    arraySum = arraySum + largeArray[i];
}
let endTime4 = now();
print "Array iteration and sum: " + str(endTime4 - startTime4) + " ms";
print "Array sum: " + str(arraySum);

// String operations benchmark
print "\n=== String Operations Performance ===";

// String concatenation
let startTime5 = now();
let longString = "";
for (let i = 0; i < 1000; i = i + 1) {
    longString = longString + "x";
}
let endTime5 = now();
print "String concatenation (1000 chars): " + str(endTime5 - startTime5) + " ms";
print "String length: " + str(len(longString));

// String building with array join
let startTime6 = now();
let stringParts = [];
for (let i = 0; i < 1000; i = i + 1) {
    stringParts.push("x");
}
let builtString = stringParts.join("");
let endTime6 = now();
print "Array-based string building: " + str(endTime6 - startTime6) + " ms";
print "Built string length: " + str(len(builtString));

// Mathematical operations benchmark
print "\n=== Mathematical Operations Performance ===";

// Square root calculations
let startTime7 = now();
let sqrtSum = 0;
for (let i = 1; i <= 1000; i = i + 1) {
    sqrtSum = sqrtSum + sqrt(float(i));
}
let endTime7 = now();
print "1000 square root calculations: " + str(endTime7 - startTime7) + " ms";
print "Sum of square roots: " + str(sqrtSum);

// Power calculations
let startTime8 = now();
let powerSum = 0;
for (let i = 1; i <= 100; i = i + 1) {
    powerSum = powerSum + pow(float(i), 2);
}
let endTime8 = now();
print "100 power calculations: " + str(endTime8 - startTime8) + " ms";
print "Sum of squares: " + str(powerSum);

// Function call overhead
print "\n=== Function Call Performance ===";

fn simpleFunction(x) {
    return x + 1;
}

let startTime9 = now();
let funcResult = 0;
for (let i = 0; i < 10000; i = i + 1) {
    funcResult = simpleFunction(funcResult);
}
let endTime9 = now();
print "10000 function calls: " + str(endTime9 - startTime9) + " ms";
print "Final result: " + str(funcResult);

// Recursive function performance
print "\n=== Recursive Function Performance ===";

fn fibonacciRecursive(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacciRecursive(n - 1) + fibonacciRecursive(n - 2);
}

let startTime10 = now();
let fibResult = fibonacciRecursive(20);
let endTime10 = now();
print "Fibonacci(20) recursive: " + str(endTime10 - startTime10) + " ms";
print "Result: " + str(fibResult);

// Iterative vs recursive comparison
fn fibonacciIterative(n) {
    if (n <= 1) {
        return n;
    }
    let a = 0;
    let b = 1;
    for (let i = 2; i <= n; i = i + 1) {
        let temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

let startTime11 = now();
let fibIterResult = fibonacciIterative(20);
let endTime11 = now();
print "Fibonacci(20) iterative: " + str(endTime11 - startTime11) + " ms";
print "Result: " + str(fibIterResult);

// Memory allocation test
print "\n=== Memory Allocation Performance ===";

let startTime12 = now();
let nestedArrays = [];
for (let i = 0; i < 100; i = i + 1) {
    let innerArray = [];
    for (let j = 0; j < 100; j = j + 1) {
        innerArray.push(i * j);
    }
    nestedArrays.push(innerArray);
}
let endTime12 = now();
print "Nested array creation (100x100): " + str(endTime12 - startTime12) + " ms";
print "Outer array length: " + str(nestedArrays.length);

// Sleep function demonstration
print "\n=== Sleep Function Demo ===";
print "Sleeping for 100ms...";
sleep(100);
print "Awake!";

print "\n=== Performance Summary ===";
print "All benchmarks completed successfully";
print "Performance testing helps identify bottlenecks";
print "Different approaches have different performance characteristics";

print "\n=== Performance Benchmarking Complete ===";