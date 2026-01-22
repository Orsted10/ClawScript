// Nested closures - Currying example
fn makeAdder(x) {
    fn add(y) {
        return x + y;
    }
    return add;
}

let add5 = makeAdder(5);
let add10 = makeAdder(10);
let add100 = makeAdder(100);

print "add5(3):";
print add5(3);

print "add5(7):";
print add5(7);

print "add10(3):";
print add10(3);

print "add100(50):";
print add100(50);
