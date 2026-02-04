// Test new array methods
let numbers = [3, 1, 4, 1, 5, 9, 2, 6];

// Test indexOf and lastIndexOf
print numbers.indexOf(1);      // Should be 1
print numbers.lastIndexOf(1);  // Should be 3

// Test sort
let sorted = numbers.sort();
print sorted;

// Test shift/unshift
let shifted = numbers.shift();
print shifted;                 // Should be 3
numbers.unshift(0);
print numbers;                 // Should start with 0

// Test flat
let nested = [[1, 2], [3, 4], 5];
let flattened = nested.flat();
print flattened;               // Should be [1, 2, 3, 4, 5]

print "All new array methods working!";