// Test array method chaining
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

// Test individual methods
doubled = numbers.map(fun(x) { return x * 2; });
print "Doubled: " + str(doubled);

evens = numbers.filter(fun(x) { return x % 2 == 0; });
print "Evens: " + str(evens);

sum = numbers.reduce(fun(acc, x) { return acc + x; }, 0);
print "Sum: " + str(sum);

// Test chaining
chained_result = numbers
    .filter(fun(x) { return x > 3; })      // [4, 5, 6, 7, 8, 9, 10]
    .map(fun(x) { return x * 2; })         // [8, 10, 12, 14, 16, 18, 20]
    .filter(fun(x) { return x % 3 == 0; }) // [12, 18]
    .reduce(fun(acc, x) { return acc + x; }, 0); // 30

print "Chained result: " + str(chained_result);

// Test slice
sliced = numbers.slice(2, 6);
print "Sliced [2:6]: " + str(sliced);

// Test find
found = numbers.find(fun(x) { return x > 7; });
print "Found > 7: " + str(found);