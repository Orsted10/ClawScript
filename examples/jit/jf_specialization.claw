print "=== JumpIfFalse Specialization ===";

let count = 0;
while (count < 5) {
  if (false) {
    print "should not see";
  } else {
    print "false: else branch";
  }
  if (nil) {
    print "should not see";
  } else {
    print "nil: else branch";
  }
  let x = 42;
  if (x) {
    print "number is truthy";
  } else {
    print "should not see";
  }
  count = count + 1;
}

print "Done.";
