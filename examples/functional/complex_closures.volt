// Complex closure example - Function factory
fn makeMultiplier(factor) {
    fn multiply(n) {
        return n * factor;
    }
    return multiply;
}

let double = makeMultiplier(2);
let triple = makeMultiplier(3);
let times10 = makeMultiplier(10);

print "Doubling:";
for (let i = 1; i <= 5; i = i + 1) {
    print double(i);
}

print "Tripling:";
for (let i = 1; i <= 5; i = i + 1) {
    print triple(i);
}

print "Times 10:";
print times10(7);
print times10(12);
