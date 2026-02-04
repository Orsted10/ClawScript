// Recursive factorial
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print "Factorial of 5:";
print factorial(5);

print "Factorial of 7:";
print factorial(7);

print "Factorial of 10:";
print factorial(10);
