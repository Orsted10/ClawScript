# VoltScript v0.8.0 Migration Guide

## Overview

This guide helps you migrate your existing VoltScript code to take advantage of the new features in v0.8.0 while maintaining compatibility with older versions.

## Backward Compatibility

✅ **Fully backward compatible** - All existing code continues to work without changes

## New Features You Can Adopt

### 1. Array Method Chaining

**Old approach:**
```volt
// Traditional approach with separate operations
let numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

// Step 1: Double each number
let doubled = [];
for (let i = 0; i < numbers.length; i++) {
    doubled.push(numbers[i] * 2);
}

// Step 2: Filter multiples of 4
let filtered = [];
for (let i = 0; i < doubled.length; i++) {
    if (doubled[i] % 4 == 0) {
        filtered.push(doubled[i]);
    }
}

// Step 3: Sum them up
let sum = 0;
for (let i = 0; i < filtered.length; i++) {
    sum += filtered[i];
}

print sum;  // 30
```

**New approach (v0.8.0):**
```volt
// Modern functional approach with method chaining
let numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
let result = numbers
    .map(fun(x) { return x * 2; })
    .filter(fun(x) { return x % 4 == 0; })
    .reduce(fun(acc, x) { return acc + x; }, 0);

print result;  // 30
```

### 2. Anonymous Functions

**Old approach:**
```volt
// Named function for callbacks
fn double(x) {
    return x * 2;
}

let numbers = [1, 2, 3, 4, 5];
let doubled = map(numbers, double);
```

**New approach (v0.8.0):**
```volt
// Anonymous function for cleaner code
let numbers = [1, 2, 3, 4, 5];
let doubled = map(numbers, fun(x) { return x * 2; });
```

### 3. Built-in Functional Utilities

**Old approach:**
```volt
// Manual implementation
fn filterEven(arr) {
    let result = [];
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] % 2 == 0) {
            result.push(arr[i]);
        }
    }
    return result;
}

let numbers = [1, 2, 3, 4, 5];
let evens = filterEven(numbers);
```

**New approach (v0.8.0):**
```volt
// Using built-in filter function
let numbers = [1, 2, 3, 4, 5];
let evens = filter(numbers, fun(x) { return x % 2 == 0; });
```

## Performance Optimization Tips

### 1. Use Method Chaining for Data Processing
```volt
// Instead of multiple loops, chain operations
// This is more readable and often more efficient
let processed = data
    .map(transform)
    .filter(validate)
    .reduce(combine, initialValue);
```

### 2. Leverage Anonymous Functions
```volt
// For simple operations, anonymous functions are cleaner
let squares = map(numbers, fun(x) { return x * x; });

// For complex operations, named functions are still better
fn complexTransformation(item) {
    // Complex logic here
    return transformedItem;
}

let results = map(data, complexTransformation);
```

### 3. Use Built-in Functions When Available
```volt
// Prefer built-in functions over manual implementations
let reversed = reverse(array);           // ✅ Preferred
// vs
let reversed = [];                       // ❌ Manual approach
for (let i = array.length - 1; i >= 0; i--) {
    reversed.push(array[i]);
}
```

## Code Modernization Examples

### Example 1: Data Processing Pipeline

**Before (v0.7.9):**
```volt
// Process student grades
let students = [
    {"name": "Alice", "grade": 85},
    {"name": "Bob", "grade": 92},
    {"name": "Charlie", "grade": 78},
    {"name": "Diana", "grade": 96}
];

// Find honor students (grade >= 90)
let honorStudents = [];
for (let i = 0; i < students.length; i++) {
    if (students[i].grade >= 90) {
        honorStudents.push(students[i].name);
    }
}

// Calculate average grade
let total = 0;
for (let i = 0; i < students.length; i++) {
    total += students[i].grade;
}
let average = total / students.length;

print "Honor students: " + str(honorStudents);
print "Class average: " + str(average);
```

**After (v0.8.0):**
```volt
// Modern approach with functional methods
let students = [
    {"name": "Alice", "grade": 85},
    {"name": "Bob", "grade": 92},
    {"name": "Charlie", "grade": 78},
    {"name": "Diana", "grade": 96}
];

// Find honor students using filter
let honorStudents = students
    .filter(fun(student) { return student.grade >= 90; })
    .map(fun(student) { return student.name; });

// Calculate average using reduce
let total = students.reduce(fun(sum, student) { 
    return sum + student.grade; 
}, 0);
let average = total / students.length;

print "Honor students: " + str(honorStudents);
print "Class average: " + str(average);
```

### Example 2: Array Transformations

**Before (v0.7.9):**
```volt
// Transform and filter data manually
let rawData = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

// Square even numbers
let squaredEvens = [];
for (let i = 0; i < rawData.length; i++) {
    if (rawData[i] % 2 == 0) {
        squaredEvens.push(rawData[i] * rawData[i]);
    }
}

// Sum the results
let sum = 0;
for (let i = 0; i < squaredEvens.length; i++) {
    sum += squaredEvens[i];
}

print sum;  // 220
```

**After (v0.8.0):**
```volt
// Clean functional approach
let rawData = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

let result = rawData
    .filter(fun(x) { return x % 2 == 0; })
    .map(fun(x) { return x * x; })
    .reduce(fun(acc, x) { return acc + x; }, 0);

print result;  // 220
```

## Best Practices for v0.8.0

### 1. Function Expression Guidelines
```volt
// ✅ Good: Simple anonymous functions
let doubled = map(numbers, fun(x) { return x * 2; });

// ✅ Good: Multi-line anonymous functions for clarity
let processed = data.map(fun(item) {
    let transformed = transform(item);
    return validate(transformed) ? transformed : null;
});

// ❌ Avoid: Complex logic in anonymous functions
let badExample = data.map(fun(item) {
    // Too much logic here - use named function instead
    complexProcessing(item);
    additionalValidation(item);
    return finalTransformation(item);
});
```

### 2. Method Chaining Best Practices
```volt
// ✅ Good: Clear, readable chains
let result = data
    .filter(isValid)
    .map(transform)
    .reduce(combine, initialValue);

// ✅ Good: Break long chains for readability
let filtered = data.filter(isValid);
let transformed = filtered.map(transform);
let result = transformed.reduce(combine, initialValue);

// ❌ Avoid: Excessively long chains
let result = data.filter(...).map(...).filter(...).map(...).reduce(...);
```

### 3. Performance Considerations
```volt
// ✅ Good: Use built-in functions for common operations
let reversed = reverse(array);

// ✅ Good: Method chaining for data pipelines
let processed = data.map(transform).filter(isValid);

// ⚠️ Caution: Be mindful of intermediate arrays
// Each chained method creates a new array
let inefficient = data
    .map(step1)
    .map(step2)  // Creates intermediate array
    .map(step3); // Creates another intermediate array

// Better approach when possible:
let efficient = data.map(fun(item) {
    return step3(step2(step1(item)));
});
```

## Testing Your Migrated Code

### 1. Verify Existing Functionality
```bash
# Run existing tests to ensure backward compatibility
./build/bin/Release/volt_tests
```

### 2. Test New Features
```volt
// Test array method chaining
let testArray = [1, 2, 3, 4, 5];
let result = testArray
    .map(fun(x) { return x * 2; })
    .filter(fun(x) { return x > 5; })
    .reduce(fun(acc, x) { return acc + x; }, 0);

assert(result == 18);  // 6 + 8 + 10 = 24? Wait, let me recalculate...

// Actually: [1,2,3,4,5] -> [2,4,6,8,10] -> [6,8,10] -> 6+8+10 = 24
assert(result == 24);
```

### 3. Performance Testing
```volt
// Compare performance between old and new approaches
let largeArray = [];
for (let i = 0; i < 10000; i++) {
    largeArray.push(i);
}

// Old approach timing
let start1 = now();
let result1 = [];
for (let i = 0; i < largeArray.length; i++) {
    if (largeArray[i] % 2 == 0) {
        result1.push(largeArray[i] * 2);
    }
}
let time1 = now() - start1;

// New approach timing
let start2 = now();
let result2 = largeArray
    .filter(fun(x) { return x % 2 == 0; })
    .map(fun(x) { return x * 2; });
let time2 = now() - start2;

print "Old approach: " + str(time1) + "ms";
print "New approach: " + str(time2) + "ms";
```

## Troubleshooting Common Issues

### Issue 1: Function Expression Not Working
```volt
// ❌ Wrong: Missing return statement
let doubler = fun(x) { x * 2; };  // Returns nil!

// ✅ Correct: Explicit return
let doubler = fun(x) { return x * 2; };
```

### Issue 2: Method Chaining Errors
```volt
// ❌ Wrong: Calling method on non-array
let notArray = "string";
let result = notArray.map(fun(x) { return x; });  // Error!

// ✅ Correct: Ensure array type
let dataArray = [1, 2, 3];
let result = dataArray.map(fun(x) { return x * 2; });
```

### Issue 3: Performance Concerns
```volt
// ❌ Inefficient: Creating many intermediate arrays
let result = data
    .map(step1)
    .map(step2)
    .map(step3)
    .map(step4);

// ✅ Better: Combine operations when possible
let result = data.map(fun(item) {
    return step4(step3(step2(step1(item))));
});
```

## Resources

- [Full Release Notes](v0.8.0_RELEASE_NOTES.md)
- [API Documentation](API_REFERENCE.md)
- [Examples Directory](../examples/)
- [Test Suite](../tests/)

---

**Happy coding with VoltScript v0.8.0! 🚀**