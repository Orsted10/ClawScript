// Test file for VoltScript Error Codes (Phase 2.2)

print "--- Testing E3001: Undefined Variable ---";
try {
    print undefinedVar;
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E2001: Type Mismatch (Arithmetic) ---";
try {
    print 10 + "string"; // Should work (coercion)
    print 10 - "string"; // Should fail
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E4001: Division by Zero ---";
try {
    print 10 / 0;
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E4002: Index Out of Bounds ---";
try {
    let arr = [1, 2, 3];
    print arr[5];
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E2002: Not Callable ---";
try {
    let notFunc = 123;
    notFunc();
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E4007: Argument Count Mismatch ---";
try {
    fn test(a, b) { return a + b; }
    test(1);
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E4003: Stack Overflow ---";
try {
    fn overflow() { overflow(); }
    overflow();
} catch (e) {
    print "Caught error: " + e;
}

print "\n--- Testing E1001: Syntax Error (via eval-like behavior if supported, or just separate test) ---";
// Syntax errors are usually caught at parse time, so we'd need to run a separate invalid file.
// But we can check if the REPL handles it.

print "\nDone testing error codes.";
