// main.volt
import { square, PI, circleArea } from "modules/math_utils.volt";

print "square(5) = " + square(5);
print "PI = " + PI;
print "circleArea(2) = " + circleArea(2);

// Test module caching - importing again should not print "Math utils module loaded!" again
import { cube } from "modules/math_utils.volt";
print "cube(3) = " + cube(3);
