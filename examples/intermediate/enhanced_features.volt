// Enhanced features demonstration for VoltScript v0.7.1

// Hash map enhancements
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};

// New has() function to check key existence
print "Has 'name': " + str(has(person, "name"));
print "Has 'email': " + str(has(person, "email"));

// New remove() function to remove key-value pairs
print "Removing 'age': " + str(remove(person, "age"));
print "Has 'age' after removal: " + str(has(person, "age"));

// Enhanced len() function now works with hash maps
print "Person size: " + len(person);

// String utility functions
let text = "Hello World";

// Convert to uppercase/lowercase
print "Upper: " + upper(text);
print "Lower: " + lower(text);

// Extract substring
print "Substring (6,5): " + substr(text, 6, 5);  // "World"

// Find substring position
print "Position of 'World': " + str(indexOf(text, "World"));  // 6
print "Position of 'xyz': " + str(indexOf(text, "xyz"));      // -1

// Chaining operations
let processed = upper(lower("  MiXeD cAsE  "));
print "Processed: '" + processed + "'";

// Edge cases
print "Empty string upper: '" + upper("") + "'";
print "Out of bounds substr: '" + substr("test", 10, 5) + "'";
print "Length of hash map: " + len({"a": 1, "b": 2, "c": 3});

print "All enhanced features working!";