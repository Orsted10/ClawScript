// Bank account with closures
fn createAccount(initialBalance) {
    let balance = initialBalance;
    
    fn getBalance() {
        return balance;
    }
    
    return getBalance;
}

let account = createAccount(100);

print "Initial balance:";
print account();

// This demonstrates closures capturing state!
// The 'balance' variable is preserved even after createAccount returns.
