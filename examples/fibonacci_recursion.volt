// Recursive Fibonacci
fn fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

print "Fibonacci sequence:";
for (let i = 0; i < 10; i = i + 1) {
    print fib(i);
}
