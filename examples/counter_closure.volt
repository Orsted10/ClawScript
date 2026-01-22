// Closure example - Counter
fn makeCounter() {
    let count = 0;
    
    fn increment() {
        count = count + 1;
        return count;
    }
    
    return increment;
}

print "Creating two independent counters:";
let counter1 = makeCounter();
let counter2 = makeCounter();

print "Counter 1:";
print counter1();  // 1
print counter1();  // 2
print counter1();  // 3

print "Counter 2:";
print counter2();  // 1
print counter2();  // 2

print "Counter 1 again:";
print counter1();  // 4
