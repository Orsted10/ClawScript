fun fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

print "Starting benchmark (Fibonacci 30)...";
let start = clock();
print fib(30);
let end = clock();
print "Time: " + (end - start) + "s";
