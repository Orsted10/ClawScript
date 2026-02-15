// Example: Integration-style JSON + File I/O pipeline (v0.9.0)
// Expected:
// - Writes a JSON file, reads it back, decodes, and prints processed results.
//
// Prerequisites:
// - File system access in the current working directory.
//
// This demonstrates interaction between core language features and
// standard library functions: jsonEncode/jsonDecode + file I/O.

let data = {
    "name": "VoltScript",
    "version": "0.9.0",
    "features": ["vm", "nan-boxing", "json", "file-io"],
    "stable": true
};

let json = jsonEncode(data);
writeFile("example_output.json", json);

let jsonBack = readFile("example_output.json");
let decoded = jsonDecode(jsonBack);

print "=== JSON File Pipeline ===";
print "Name: " + decoded["name"];
print "Version: " + decoded["version"];
print "Feature count: " + str(decoded["features"].length);
print "Is stable: " + str(decoded["stable"]);

// Example output:
// === JSON File Pipeline ===
// Name: VoltScript
// Version: 0.9.0
// Feature count: 4
// Is stable: true

