fun add(a, b) { return a + b; }
let i = 0;
while (i < 10) {
  let r = add(1, 2);
  if (r != 3) { print "wrong"; }
  i = i + 1;
}
print "call binding done";
