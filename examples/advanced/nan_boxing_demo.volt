// Example: NaN-boxed behavior is invisible at the language level (v0.9.0)
// Expected: Demonstrates dynamic typing and mixed data in arrays and hash maps.
//
// Prerequisites:
// - None beyond a standard VoltScript build.
//
// Although the runtime uses NaN-boxing internally, from VoltScript you just
// work with numbers, strings, booleans, nil, arrays, hash maps, and functions.

print "=== Dynamic Values Demo (backed by NaN-boxing) ===";

let values = [
    42,
    3.14,
    "hello",
    true,
    nil,
    {"key": "value"},
    fun(x) { return x * 2; }
];

print "Length: " + str(values.length);
print "Type[0]: " + type(values[0]);
print "Type[2]: " + type(values[2]);
print "Type[4]: " + type(values[4]);
print "Type[5]: " + type(values[5]);

let doubled = values[6](21);
print "Function call result: " + str(doubled);

// Example output:
// === Dynamic Values Demo (backed by NaN-boxing) ===
// Length: 7
// Type[0]: number
// Type[2]: string
// Type[4]: nil
// Type[5]: hashmap
// Function call result: 42

