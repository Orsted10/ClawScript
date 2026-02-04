// String manipulation with functions
fn repeat(str, times) {
    let result = "";
    for (let i = 0; i < times; i = i + 1) {
        result = result + str;
    }
    return result;
}

fn makeGreeter(greeting) {
    fn greet(name) {
        print greeting + ", " + name + "!";
    }
    return greet;
}

print repeat("Hello ", 3);
print repeat("*", 10);

let casualGreet = makeGreeter("Hey");
let formalGreet = makeGreeter("Good evening");

casualGreet("Alice");
formalGreet("Mr. Smith");
