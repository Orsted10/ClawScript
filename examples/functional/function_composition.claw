// Function composition
fn compose(f, g, x) {
    return f(g(x));
}

fn double(n) {
    return n * 2;
}

fn addTen(n) {
    return n + 10;
}

fn square(n) {
    return n * n;
}

print "double(addTen(5)):";
print compose(double, addTen, 5);

print "square(double(4)):";
print compose(square, double, 4);

print "addTen(square(3)):";
print compose(addTen, square, 3);
