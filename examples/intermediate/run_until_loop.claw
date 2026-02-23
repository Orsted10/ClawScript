// Basic run-until loop example
let i = 0;
run {
    print i;
    i = i + 1;
} until (i == 5);

print "---";

// Fibonacci with run-until
let a = 0;
let b = 1;
run {
    print a;
    let temp = a + b;
    a = b;
    b = temp;
} until (a >= 100);

print "---";

// Countdown example
let countdown = 5;
run {
    print countdown;
    countdown--;
} until (countdown <= 0);
print "Liftoff!";
