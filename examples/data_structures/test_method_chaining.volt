// Test method chaining functionality
let numbers = [1, 2, 3, 4, 5];

// Test map method
let doubled = numbers.map(fun(x) { return x * 2; });
print doubled;

// Test filter method
let evens = numbers.filter(fun(x) { return x % 2 == 0; });
print evens;

// Test reduce method
let sum = numbers.reduce(fun(acc, x) { return acc + x; }, 0);
print sum;

// Test chaining
let result = numbers
    .filter(fun(x) { return x > 2; })
    .map(fun(x) { return x * x; })
    .reduce(fun(acc, x) { return acc + x; }, 0);
print result;

// Test slice method
let sliced = numbers.slice(1, 4);
print sliced;

// Test find method
let found = numbers.find(fun(x) { return x > 3; });
print found;

// Test some/every methods
let hasEven = numbers.some(fun(x) { return x % 2 == 0; });
let allPositive = numbers.every(fun(x) { return x > 0; });
print hasEven;
print allPositive;

print "All tests passed!";