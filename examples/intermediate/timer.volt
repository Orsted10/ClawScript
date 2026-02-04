// Using the built-in clock() function
fn benchmark(f, iterations) {
    let start = clock();
    
    for (let i = 0; i < iterations; i = i + 1) {
        f(i);
    }
    
    let end = clock();
    let elapsed = end - start;
    
    print "Time elapsed (seconds):";
    print elapsed;
}

fn dummy(n) {
    let result = n * n;
    return result;
}

print "Benchmarking 1000 operations:";
benchmark(dummy, 1000);
