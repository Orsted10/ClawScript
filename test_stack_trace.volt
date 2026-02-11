// Test stack trace with deep call chain
fn a() {
    print("In a");
    b();
}

fn b() {
    print("In b");
    c();
}

fn c() {
    print("In c, about to cause error");
    10 / 0; // Division by zero
}

print("Starting stack trace test...");
a();
