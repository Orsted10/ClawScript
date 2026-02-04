// VoltScript v0.7.5 Demo
// Demonstrating new features

print "=== VoltScript v0.7.5 Demo ===";

// Test trigonometric functions
print "Trigonometric functions:";
print "sin(0) = " + str(sin(0));
print "cos(0) = " + str(cos(0));
print "tan(0) = " + str(tan(0));

// Test logarithmic functions
print "\nLogarithmic functions:";
print "log(1) = " + str(log(1));
print "log(e) = " + str(log(2.718281828));
print "exp(0) = " + str(exp(0));
print "exp(1) = " + str(exp(1));

// Test date/time functions
print "\nDate/Time functions:";
let timestamp = now();
print "Current timestamp: " + str(timestamp);
print "Formatted date: " + formatDate(timestamp, "yyyy-MM-dd");

// Test JSON functions
print "\nJSON functions:";
let testData = {"name": "John", "age": 30, "active": true};
let jsonStr = jsonEncode(testData);
print "Encoded JSON: " + jsonStr;

let jsonArray = [1, 2, 3, "hello"];
let jsonArrStr = jsonEncode(jsonArray);
print "Encoded array: " + jsonArrStr;

let decoded = jsonDecode("{\"test\": 123}");
print "Decoded type: " + type(decoded);

print "\nDemo completed!";