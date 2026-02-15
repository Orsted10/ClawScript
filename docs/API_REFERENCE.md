# VoltScript v0.9.0 API Reference

This reference describes the core standard library available in VoltScript v0.9.0.
It is based on the v0.8.6 APIs and extended with the features introduced in v0.9.0
such as the class system, JSON engine, and performance tools.

## Table of Contents
1. [Global Functions](#global-functions)
2. [Array Methods](#array-methods)
3. [Hash Map Methods](#hash-map-methods)
4. [String Functions](#string-functions)
5. [Math Functions](#math-functions)
6. [File I/O Functions](#file-io-functions)
7. [Date/Time Functions](#datetime-functions)
8. [JSON Functions](#json-functions)
9. [Class System](#class-system)
10. [Functional Utilities](#functional-utilities)
11. [Performance Tools](#performance-tools)

## Global Functions

### Control Flow
```volt
print(value)                    // Print value to console
exit()                          // Exit program
type(value)                     // Get type of value as string
```

### Variable Operations
```volt
let name = value               // Declare variable
name = newValue                // Reassign variable
```

### Control Structures
```volt
if (condition) { ... }         // Conditional execution
if (condition) { ... } else { ... }
while (condition) { ... }      // Loop while condition is true
for (init; condition; increment) { ... }  // C-style for loop
run { ... } until (condition)  // Execute at least once
break                          // Exit loop
continue                       // Skip to next iteration
return value                   // Return from function
```

## Array Methods

### Properties
```volt
array.length                   // Number of elements in array
```

### Mutation Methods
```volt
array.push(value)              // Add element to end, returns nil
array.pop()                    // Remove and return last element
array.reverse()                // Reverse array in-place, returns nil
```

### Functional Methods
```volt
array.map(function)            // Transform each element
// Example: [1,2,3].map(fun(x) { return x * 2; }) → [2,4,6]

array.filter(function)         // Select elements matching predicate
// Example: [1,2,3,4].filter(fun(x) { return x % 2 == 0; }) → [2,4]

array.reduce(function, initial) // Combine elements into single value
// Example: [1,2,3].reduce(fun(acc, x) { return acc + x; }, 0) → 6

array.join(separator)          // Join elements into string
// Example: [1,2,3].join("-") → "1-2-3"

array.concat(otherArray)       // Combine two arrays
// Example: [1,2].concat([3,4]) → [1,2,3,4]

array.slice(start, end)        // Extract portion of array
// Example: [1,2,3,4,5].slice(1, 4) → [2,3,4]

array.flat()                   // Flatten nested arrays
// Example: [[1,2],[3,4]].flat() → [1,2,3,4]

array.flatMap(function)        // Map then flatten results
// Example: [1,2].flatMap(fun(x) { return [x, x*2]; }) → [1,2,2,4]
```

## Hash Map Methods

### Properties
```volt
hashMap.size                   // Number of key-value pairs
```

### Access Methods
```volt
hashMap.keys()                 // Get array of all keys
hashMap.values()               // Get array of all values
hashMap.has(key)               // Check if key exists
hashMap.remove(key)            // Remove key-value pair
```

### Global Functions for Hash Maps
```volt
keys(hashMap)                  // Get array of keys
values(hashMap)                // Get array of values
has(hashMap, key)              // Check if key exists
remove(hashMap, key)           // Remove key-value pair and return value
```

## String Functions

### Basic Operations
```volt
len(string)                    // Get string length
str(value)                     // Convert value to string
toUpper(string)                // Convert to uppercase
toLower(string)                // Convert to lowercase
upper(string)                  // Alias for toUpper
lower(string)                  // Alias for toLower
```

### Substring Operations
```volt
substr(string, start, length)  // Extract substring
indexOf(string, substring)     // Find substring position
```

### Formatting
```volt
trim(string)                   // Remove whitespace from ends
padStart(string, length, pad)  // Pad string at start
padEnd(string, length, pad)    // Pad string at end
repeat(string, count)          // Repeat string
```

### Splitting and Joining
```volt
split(string, delimiter)       // Split string into array
replace(string, search, replacement)  // Replace substring
```

### Pattern Matching
```volt
startsWith(string, prefix)     // Check if string starts with prefix
endsWith(string, suffix)       // Check if string ends with suffix
```

### Character Operations
```volt
charCodeAt(string, index)      // Get character code at index
fromCharCode(code)             // Create string from character code
```

## Math Functions

### Basic Operations
```volt
abs(x)                         // Absolute value
sqrt(x)                        // Square root
pow(base, exponent)            // Power function
min(a, b)                      // Minimum of two values
max(a, b)                      // Maximum of two values
```

### Rounding
```volt
round(x)                       // Round to nearest integer
floor(x)                       // Round down
ceil(x)                        // Round up
```

### Trigonometry
```volt
sin(x)                         // Sine (radians)
cos(x)                         // Cosine (radians)
tan(x)                         // Tangent (radians)
```

### Logarithms and Exponents
```volt
log(x)                         // Natural logarithm
exp(x)                         // Exponential (e^x)
```

### Random Numbers
```volt
random()                       // Random float between 0 and 1
```

## File I/O Functions

### File Operations
```volt
readFile(path)                 // Read entire file as string
writeFile(path, content)       // Write content to file
appendFile(path, content)      // Append content to file
fileExists(path)               // Check if file exists
exists(path)                   // Alias for fileExists
deleteFile(path)               // Delete file
fileSize(path)                 // Get file size in bytes
```

## Date/Time Functions

```volt
now()                          // Current timestamp in milliseconds
formatDate(timestamp, format)  // Format timestamp (placeholder)
```

## JSON Functions

```volt
jsonEncode(value)              // Convert value to JSON string
jsonDecode(jsonString)         // Parse JSON string to value
```

## Class System

### Class Declaration
```volt
class Name {
    init(args) { ... }         // Constructor
    method() { ... }           // Instance method
}
```

### Inheritance
```volt
class Child extends Parent {
    method() {
        super.method();        // Call parent method
    }
}
```

### Instance Operations
```volt
let instance = ClassName(args) // Create instance
instance.property = value      // Set property
print instance.property        // Get property
instance.method()              // Call method
```

## Functional Utilities

### Function Composition
```volt
compose(f1, f2, ...)           // Compose functions (right to left)
// Example: compose(f, g)(x) = f(g(x))

pipe(f1, f2, ...)              // Pipe value through functions (left to right)
// Example: pipe(f, g)(x) = g(f(x))
```

### Array Transformation
```volt
map(array, function)           // Apply function to each element
// Example: map([1,2,3], fun(x) { return x * 2; }) → [2,4,6]

filter(array, function)        // Select elements matching predicate
// Example: filter([1,2,3,4], fun(x) { return x % 2 == 0; }) → [2,4]

reverse(array)                 // Reverse array elements
// Example: reverse([1,2,3]) → [3,2,1]
```

## Performance Tools

```volt
sleep(milliseconds)            // Pause execution
benchmark(function, ...args)   // Measure execution time
// Returns object with timeMicroseconds property
```

## Operators

### Arithmetic
```volt
+ - * / %                      // Basic arithmetic
+= -= *= /= %=                 // Compound assignment
++ --                          // Increment/decrement
```

### Comparison
```volt
== != < > <= >=                // Comparison operators
```

### Logical
```volt
&& || !                        // Logical operators
? :                            // Ternary operator
```

### Data Access
```volt
array[index]                   // Array indexing
hashMap[key]                   // Hash map access
object.property                // Property access
```

## Data Types

### Primitive Types
```volt
nil                            // Null value
true/false                     // Boolean values
42, 3.14                       // Number literals
"hello"                        // String literals
```

### Collection Types
```volt
[1, 2, 3]                      // Array literal
{"key": "value"}               // Hash map literal
```

### Function Types
```volt
fn name(params) { body }       // Function declaration
fun(params) { body }           // Anonymous function expression
```

## Error Handling

### Runtime Errors
Errors are thrown for various conditions:
- Division by zero
- Invalid array indices
- Undefined variables
- Type mismatches
- File operation failures

### Error Information
Errors include:
- Error message
- Line number
- Column number
- Source file information

## Best Practices

### Naming Conventions
- Use `camelCase` for variables and functions
- Use `PascalCase` for constructors (future feature)
- Use descriptive names that indicate purpose

### Function Design
```volt
// ✅ Good: Pure functions with clear parameters
fn calculateArea(width, height) {
    return width * height;
}

// ✅ Good: Functions that do one thing well
fn isValidEmail(email) {
    return indexOf(email, "@") != -1;
}
```

### Array Operations
```volt
// ✅ Good: Use functional methods for data transformation
let doubled = numbers.map(fun(x) { return x * 2; });

// ✅ Good: Chain operations for readability
let result = data
    .filter(isValid)
    .map(transform)
    .reduce(combine, 0);
```

### Error Prevention
```volt
// ✅ Good: Check preconditions
fn divide(a, b) {
    if (b == 0) {
        print "Error: Division by zero";
        return nil;
    }
    return a / b;
}

// ✅ Good: Validate input types
fn processArray(arr) {
    if (type(arr) != "array") {
        print "Error: Expected array";
        return nil;
    }
    // Process array...
}
```

## Examples

### Complete Program Structure
```volt
// Import statements (future feature)
// Main function
fn main() {
    // Program logic here
    print "Hello, World!";
}

// Call main function
main();
```

### Data Processing Pipeline
```volt
// Process sales data
let sales = [
    {"product": "A", "amount": 100},
    {"product": "B", "amount": 150},
    {"product": "A", "amount": 200}
];

// Calculate total sales by product
let totals = sales
    .filter(fun(sale) { return sale.amount > 100; })
    .reduce(fun(acc, sale) {
        if (!has(acc, sale.product)) {
            acc[sale.product] = 0;
        }
        acc[sale.product] += sale.amount;
        return acc;
    }, {});

print totals;  // {"B": 150, "A": 200}
```

### File Processing
```volt
// Read and process file
if (exists("data.txt")) {
    let content = readFile("data.txt");
    let lines = split(content, "\n");
    let processed = lines
        .map(trim)
        .filter(fun(line) { return len(line) > 0; });
    
    print "Processed " + str(processed.length) + " lines";
} else {
    print "File not found";
}
```

---

**Last Updated:** February 12, 2026  
**Version:** 0.8.6
