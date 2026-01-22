// Find max and min using functions
fn max(a, b) {
    if (a > b) return a;
    return b;
}

fn min(a, b) {
    if (a < b) return a;
    return b;
}

fn maxOfThree(a, b, c) {
    return max(a, max(b, c));
}

fn minOfThree(a, b, c) {
    return min(a, min(b, c));
}

print "max(5, 10):";
print max(5, 10);

print "min(5, 10):";
print min(5, 10);

print "maxOfThree(3, 7, 5):";
print maxOfThree(3, 7, 5);

print "minOfThree(3, 7, 5):";
print minOfThree(3, 7, 5);
