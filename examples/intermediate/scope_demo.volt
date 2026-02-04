// Variable shadowing
let x = "global";
print x;

{
    let x = "block 1";
    print x;
    
    {
        let x = "block 2";
        print x;
    }
    
    print x;
}

print x;
