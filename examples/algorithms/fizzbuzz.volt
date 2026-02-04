// FizzBuzz
for (let i = 1; i <= 15; i = i + 1) {
    let mod3 = i % 3;
    let mod5 = i % 5;
    
    if (mod3 == 0 && mod5 == 0) {
        print "FizzBuzz";
    } else if (mod3 == 0) {
        print "Fizz";
    } else if (mod5 == 0) {
        print "Buzz";
    } else {
        print i;
    }
}
