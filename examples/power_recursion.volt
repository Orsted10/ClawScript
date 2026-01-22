// Calculate power recursively
fn power(base, exponent) {
    if (exponent == 0) {
        return 1;
    }
    if (exponent == 1) {
        return base;
    }
    return base * power(base, exponent - 1);
}

print "2^5:";
print power(2, 5);

print "3^4:";
print power(3, 4);

print "5^3:";
print power(5, 3);

print "10^2:";
print power(10, 2);
