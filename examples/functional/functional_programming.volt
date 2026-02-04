// Example: Functional Programming in VoltScript
print("=== Functional Programming Demo ===");

// Define some basic functions
add = fun(a, b) { return a + b; };
multiply = fun(a, b) { return a * b; };
square = fun(x) { return x * x; };
double = fun(x) { return x * 2; };
increment = fun(x) { return x + 1; };

// Using compose to combine functions
addThenSquare = compose(square, add);  // Square the result of addition
result1 = addThenSquare(3, 4);  // square(add(3, 4)) = square(7) = 49
print("Compose result (square of 3+4): " + str(result1));

// Another composition example
doubleThenIncrement = compose(increment, double);  // Increment the result of doubling
result2 = doubleThenIncrement(5);  // increment(double(5)) = increment(10) = 11
print("Compose result (increment of double 5): " + str(result2));

// Using pipe to chain functions
squareThenDouble = pipe(square, double);  // Double the result of squaring
result3 = squareThenDouble(3);  // double(square(3)) = double(9) = 18
print("Pipe result (double of square 3): " + str(result3));

// More complex pipeline
complexPipeline = pipe(increment, square, double);  // double(square(increment(x)))
result4 = complexPipeline(2);  // double(square(increment(2))) = double(square(3)) = double(9) = 18
print("Complex pipeline result: " + str(result4));

// Real-world example: Processing a list of numbers
numbers = [1, 2, 3, 4, 5];

// Create a function to process each number: increment, then square, then double
processNumber = pipe(increment, square, double);

// Apply the processing function to each number
processedNumbers = [];
for (let i = 0; i < numbers.length; i = i + 1) {
    processedNumbers.push(processNumber(numbers[i]));
}

print("Original numbers: [1, 2, 3, 4, 5]");
print("Processed numbers: [");
for (let i = 0; i < processedNumbers.length; i = i + 1) {
    print("  " + str(processedNumbers[i]) + ",");
}
print("]");

// Function factories using closures
createMultiplier = fun(factor) {
    return fun(value) {
        return value * factor;
    };
};

doubler = createMultiplier(2);
tripler = createMultiplier(3);

print("Doubler(10): " + str(doubler(10)));
print("Tripler(10): " + str(tripler(10)));

// Combining functional concepts
createProcessor = fun(steps) {
    return pipe(steps[0], steps[1], steps[2]);  // Assume 3 steps for this example
};

mySteps = [increment, square, double];
myProcessor = createProcessor(mySteps);
result5 = myProcessor(4);  // double(square(increment(4))) = double(square(5)) = double(25) = 50
print("Dynamic processor result: " + str(result5));