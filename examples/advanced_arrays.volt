// Advanced array operations and methods demonstration
print "=== Advanced Array Operations ===";

// Create arrays with different data types
let numbers = [1, 2, 3, 4, 5];
let strings = ["apple", "banana", "cherry"];
let mixed = [1, "hello", true, 3.14];

print "Numbers array: " + str(numbers);
print "Strings array: " + str(strings);
print "Mixed array: " + str(mixed);

// Array properties
print "Length of numbers: " + str(numbers.length);
print "Length of strings: " + str(strings.length);

// Array methods
print "\n=== Array Methods ===";

// Push elements
numbers.push(6);
numbers.push(7);
print "After pushing 6 and 7: " + str(numbers);

// Pop elements
let last = numbers.pop();
print "Popped element: " + str(last);
print "Array after pop: " + str(numbers);

// Reverse array
numbers.reverse();
print "Reversed numbers: " + str(numbers);

// Nested arrays
let matrix = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
];

print "\n=== Matrix Operations ===";
print "Matrix: " + str(matrix);
print "Element at [1][2]: " + str(matrix[1][2]);
print "First row: " + str(matrix[0]);

// Array iteration
print "\n=== Array Iteration ===";
print "Iterating through numbers:";
for (let i = 0; i < numbers.length; i = i + 1) {
    print "numbers[" + str(i) + "] = " + str(numbers[i]);
}

// Array with mathematical operations
let squares = [];
for (let i = 1; i <= 5; i = i + 1) {
    squares.push(i * i);
}
print "Squares: " + str(squares);

// Array filtering concept
print "\n=== Even Numbers ===";
let evens = [];
for (let i = 0; i < numbers.length; i = i + 1) {
    if (numbers[i] % 2 == 0) {
        evens.push(numbers[i]);
    }
}
print "Even numbers from array: " + str(evens);

// Array sum
let sum = 0;
for (let i = 0; i < numbers.length; i = i + 1) {
    sum = sum + numbers[i];
}
print "Sum of all numbers: " + str(sum);

// Array maximum
let max = numbers[0];
for (let i = 1; i < numbers.length; i = i + 1) {
    if (numbers[i] > max) {
        max = numbers[i];
    }
}
print "Maximum number: " + str(max);

print "\n=== Array Operations Complete ===";