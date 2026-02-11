fn recursive(n) {
    recursive(n + 1);
}

print("Starting stack overflow test (limit 100)...");
recursive(1);
