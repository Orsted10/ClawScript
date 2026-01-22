// Greatest Common Divisor using Euclidean algorithm
fn gcd(a, b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

print "gcd(48, 18):";
print gcd(48, 18);

print "gcd(100, 35):";
print gcd(100, 35);

print "gcd(17, 19):";
print gcd(17, 19);
