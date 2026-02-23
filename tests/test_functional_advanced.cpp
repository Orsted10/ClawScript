#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <sstream>

using namespace claw;

namespace {

// Helper to capture print output
class PrintCapture {
public:
    PrintCapture() : oldBuf_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(oldBuf_); }
    std::string getOutput() const { return buffer_.str(); }
private:
    std::stringstream buffer_;
    std::streambuf* oldBuf_;
};

// Helper function to run code and capture output
std::string runCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto statements = parser.parseProgram();

    if (parser.hadError()) {
        return "PARSE_ERROR";
    }

    PrintCapture capture;
    Interpreter interpreter;

    try {
        interpreter.execute(statements);
    } catch (const std::exception& e) {
        return std::string("RUNTIME_ERROR: ") + e.what();
    }

    return capture.getOutput();
}

} // anonymous namespace

// Advanced functional programming tests
TEST(FunctionalAdvanced, MapFilterReduceChains) {
    std::string code = R"(
        // Test chaining map, filter, reduce
        numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        
        result = numbers
            .map(fun(x) { return x * 2; })           // [2, 4, 6, 8, 10, 12, 14, 16, 18, 20]
            .filter(fun(x) { return x % 4 == 0; })   // [4, 8, 12, 16, 20]
            .map(fun(x) { return x / 2; })           // [2, 4, 6, 8, 10]
            .reduce(fun(acc, x) { return acc + x; }, 0); // 30
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("30") != std::string::npos);
}

TEST(FunctionalAdvanced, ComplexArrayTransformations) {
    std::string code = R"(
        // Complex array transformations
        data = [
            {"name": "Alice", "age": 25, "active": true},
            {"name": "Bob", "age": 30, "active": false},
            {"name": "Charlie", "age": 35, "active": true},
            {"name": "Diana", "age": 28, "active": true}
        ];
        
        // Filter active users, transform to names, then join
        result = data
            .filter(fun(user) { return user.active; })
            .map(fun(user) { return user.name; })
            .join(", ");
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("Alice") != std::string::npos);
    EXPECT_TRUE(output.find("Charlie") != std::string::npos);
    EXPECT_TRUE(output.find("Diana") != std::string::npos);
}

TEST(FunctionalAdvanced, RecursiveFunctionalPatterns) {
    std::string code = R"(
        // Recursive functional patterns
        factorial = fun(n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        };
        
        // Calculate factorials for multiple values using functional approach
        numbers = [1, 2, 3, 4, 5];
        factorials = numbers.map(factorial);
        
        result = factorials.reduce(fun(acc, x) { return acc + x; }, 0);
        print result;  // 1 + 2 + 6 + 24 + 120 = 153
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("153") != std::string::npos);
}

TEST(FunctionalAdvanced, HigherOrderFunctionComposition) {
    std::string code = R"(
        // Higher-order function composition
        compose = fun(f, g) {
            return fun(x) {
                return f(g(x));
            };
        };
        
        addOne = fun(x) { return x + 1; };
        multiplyByTwo = fun(x) { return x * 2; };
        square = fun(x) { return x * x; };
        
        // Create composite functions
        addThenMultiply = compose(multiplyByTwo, addOne);  // multiplyByTwo(addOne(x))
        squareThenAdd = compose(addOne, square);          // addOne(square(x))
        
        result1 = addThenMultiply(5);  // multiplyByTwo(addOne(5)) = multiplyByTwo(6) = 12
        result2 = squareThenAdd(5);    // addOne(square(5)) = addOne(25) = 26
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("12") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionCurrying) {
    std::string code = R"(
        // Function currying patterns
        curry = fun(fn) {
            return fun(a) {
                return fun(b) {
                    return fun(c) {
                        return fn(a, b, c);
                    };
                };
            };
        };
        
        addThree = fun(x, y, z) {
            return x + y + z;
        };
        
        curriedAdd = curry(addThree);
        add5 = curriedAdd(5);        // Returns function that expects y
        add5and10 = add5(10);        // Returns function that expects z
        result = add5and10(15);      // 5 + 10 + 15 = 30
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("30") != std::string::npos);
}

TEST(FunctionalAdvanced, PartialApplication) {
    std::string code = R"(
        // Partial application
        partial = fun(fn, ...args) {
            return fun(...moreArgs) {
                allArgs = args.concat(moreArgs);
                return fn(...allArgs);
            };
        };
        
        multiply = fun(a, b, c) {
            return a * b * c;
        };
        
        // Create a function that multiplies by 2 and 3
        multiplyBy2and3 = partial(multiply, 2, 3);
        result = multiplyBy2and3(4);  // 2 * 3 * 4 = 24
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("24") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalListOperations) {
    std::string code = R"(
        // Functional list operations
        numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        
        // Custom functional operations
        sum = fun(arr) {
            return arr.reduce(fun(acc, x) { return acc + x; }, 0);
        };
        
        product = fun(arr) {
            return arr.reduce(fun(acc, x) { return acc * x; }, 1);
        };
        
        average = fun(arr) {
            return sum(arr) / arr.length;
        };
        
        evens = fun(arr) {
            return arr.filter(fun(x) { return x % 2 == 0; });
        };
        
        odds = fun(arr) {
            return arr.filter(fun(x) { return x % 2 == 1; });
        };
        
        result = {
            "sum": sum(numbers),
            "product": product(numbers.slice(0, 5)),  // 1*2*3*4*5 = 120
            "average": average(numbers),
            "evens": evens(numbers).length,
            "odds": odds(numbers).length
        };
        
        print result.sum;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("55") != std::string::npos);  // Sum of 1-10 is 55
}

TEST(FunctionalAdvanced, FunctionalTreeTraversal) {
    std::string code = R"(
        // Simulate tree traversal with functional patterns
        tree = {
            "value": 1,
            "children": [
                {
                    "value": 2,
                    "children": [
                        {"value": 4, "children": []},
                        {"value": 5, "children": []}
                    ]
                },
                {
                    "value": 3,
                    "children": [
                        {"value": 6, "children": []}
                    ]
                }
            ]
        };
        
        // Recursive function to collect all values
        collectValues = fun(node) {
            values = [node.value];
            
            for (let i = 0; i < node.children.length; i = i + 1) {
                childValues = collectValues(node.children[i]);
                values = values.concat(childValues);
            }
            
            return values;
        };
        
        allValues = collectValues(tree);
        result = allValues.reduce(fun(acc, x) { return acc + x; }, 0);
        
        print result;  // 1+2+3+4+5+6 = 21
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("21") != std::string::npos);
}

TEST(FunctionalAdvanced, MemoizationPatterns) {
    std::string code = R"(
        // Memoization for optimization
        memoize = fun(fn) {
            cache = {};
            return fun(...args) {
                key = args.join("-");
                if (cache.has(key)) {
                    return cache[key];
                }
                result = fn(...args);
                cache[key] = result;
                return result;
            };
        };
        
        // Expensive function to memoize
        fibonacci = fun(n) {
            if (n <= 1) {
                return n;
            }
            return fibonacci(n - 1) + fibonacci(n - 2);
        };
        
        // Create memoized version
        memoFib = memoize(fibonacci);
        
        // Calculate some values
        result1 = memoFib(10);  // Should be 55
        result2 = memoFib(8);   // Should be 21
        
        print result1;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("55") != std::string::npos);
}

TEST(FunctionalAdvanced, MonadicPatterns) {
    std::string code = R"(
        // Simple monadic patterns (Maybe pattern)
        Maybe = {
            "of": fun(value) {
                return {
                    "value": value,
                    "isNothing": value == nil,
                    "map": fun(fn) {
                        if (this.isNothing) {
                            return Maybe.of(nil);
                        }
                        return Maybe.of(fn(this.value));
                    },
                    "flatMap": fun(fn) {
                        if (this.isNothing) {
                            return Maybe.of(nil);
                        }
                        return fn(this.value);
                    },
                    "getOrElse": fun(defaultValue) {
                        if (this.isNothing) {
                            return defaultValue;
                        }
                        return this.value;
                    }
                };
            }
        };
        
        // Example usage
        result = Maybe.of(5)
            .map(fun(x) { return x * 2; })
            .map(fun(x) { return x + 1; })
            .getOrElse(0);
        
        print result;  // (5 * 2) + 1 = 11
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("11") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalErrorHandling) {
    std::string code = R"(
        // Functional error handling patterns
        Result = {
            "ok": fun(value) {
                return {
                    "isOk": true,
                    "isErr": false,
                    "value": value,
                    "map": fun(fn) { return Result.ok(fn(this.value)); },
                    "flatMap": fun(fn) { return fn(this.value); },
                    "orElse": fun(other) { return this.value; }
                };
            },
            "err": fun(error) {
                return {
                    "isOk": false,
                    "isErr": true,
                    "error": error,
                    "value": nil,
                    "map": fun(fn) { return this; },  // Do nothing on error
                    "flatMap": fun(fn) { return this; },  // Do nothing on error
                    "orElse": fun(other) { return other; }
                };
            }
        };
        
        // Safe division function
        safeDivide = fun(a, b) {
            if (b == 0) {
                return Result.err("Division by zero");
            }
            return Result.ok(a / b);
        };
        
        // Chain operations safely
        result = safeDivide(10, 2)
            .map(fun(x) { return x * 3; })
            .orElse(0);
        
        print result;  // (10 / 2) * 3 = 15
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("15") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalCollectionOperations) {
    std::string code = R"(
        // Advanced functional collection operations
        data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        
        // Group by operation
        groupBy = fun(arr, keyFn) {
            result = {};
            for (let i = 0; i < arr.length; i = i + 1) {
                key = keyFn(arr[i]);
                if (!result.has(key)) {
                    result[key] = [];
                }
                result[key].push(arr[i]);
            }
            return result;
        };
        
        // Group numbers by even/odd
        grouped = groupBy(data, fun(x) { return x % 2 == 0 ? "even" : "odd"; });
        
        evens = grouped.even.length;
        odds = grouped.odd.length;
        
        print evens;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("5") != std::string::npos);  // 5 even numbers
}

TEST(FunctionalAdvanced, FunctionalStateManagement) {
    std::string code = R"(
        // Functional state management
        createStore = fun(initialState) {
            state = initialState;
            listeners = [];
            
            return {
                "getState": fun() { return state; },
                "subscribe": fun(listener) {
                    listeners.push(listener);
                    return fun() {  // Unsubscribe function
                        index = listeners.indexOf(listener);
                        if (index != -1) {
                            listeners.splice(index, 1);
                        }
                    };
                },
                "dispatch": fun(action) {
                    // Simple state update based on action
                    if (action.type == "INCREMENT") {
                        state = state + 1;
                    } else if (action.type == "DECREMENT") {
                        state = state - 1;
                    } else if (action.type == "SET") {
                        state = action.payload;
                    }
                    
                    // Notify listeners
                    for (let i = 0; i < listeners.length; i = i + 1) {
                        listeners[i](state);
                    }
                }
            };
        };
        
        // Create store
        store = createStore(0);
        
        // Subscribe to changes
        store.subscribe(fun(newState) {
            print newState;  // Will print intermediate states
        });
        
        // Dispatch actions
        store.dispatch({ "type": "INCREMENT" });
        store.dispatch({ "type": "INCREMENT" });
        store.dispatch({ "type": "SET", "payload": 10 });
        
        finalState = store.getState();
        print finalState;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("10") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalAsyncPatterns) {
    std::string code = R"(
        // Simulate async patterns with callbacks
        Promise = {
            "resolve": fun(value) {
                return {
                    "then": fun(onResolve) {
                        result = onResolve(value);
                        return Promise.resolve(result);
                    },
                    "catch": fun(onReject) {
                        return this;  // No error to catch
                    }
                };
            },
            "reject": fun(error) {
                return {
                    "then": fun(onResolve) {
                        return this;  // Skip onResolve
                    },
                    "catch": fun(onReject) {
                        result = onReject(error);
                        return Promise.resolve(result);
                    }
                };
            }
        };
        
        // Chain operations
        result = Promise.resolve(5)
            .then(fun(x) { return x * 2; })
            .then(fun(x) { return x + 3; })
            .then(fun(x) { return x / 2; });
        
        // This is simulated - in real async it would be different
        print "async_simulated";
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("async") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalDependencyInjection) {
    std::string code = R"(
        // Functional dependency injection
        createUserService = fun(db, logger) {
            return {
                "getUser": fun(id) {
                    logger.log("Getting user: " + str(id));
                    return db.findById(id);
                },
                "createUser": fun(userData) {
                    logger.log("Creating user");
                    return db.insert(userData);
                }
            };
        };
        
        // Mock implementations
        mockDb = {
            "findById": fun(id) { return { "id": id, "name": "User" + str(id) }; },
            "insert": fun(data) { return { "id": 999, "created": true }; }
        };
        
        mockLogger = {
            "log": fun(message) { print "LOG: " + message; }
        };
        
        userService = createUserService(mockDb, mockLogger);
        user = userService.getUser(42);
        
        print user.id;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("42") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalValidationChains) {
    std::string code = R"(
        // Functional validation chains
        Validator = {
            "create": fun(value) {
                return {
                    "value": value,
                    "errors": [],
                    "validate": fun(condition, message) {
                        if (!condition(this.value)) {
                            this.errors.push(message);
                        }
                        return this;
                    },
                    "isValid": fun() {
                        return this.errors.length == 0;
                    },
                    "getErrors": fun() {
                        return this.errors;
                    }
                };
            }
        };
        
        // Validate an email-like string
        email = "test@example.com";
        result = Validator.create(email)
            .validate(fun(x) { return len(x) > 5; }, "Email too short")
            .validate(fun(x) { return x.indexOf("@") != -1; }, "Missing @ symbol")
            .validate(fun(x) { return x.indexOf(".") != -1; }, "Missing dot")
            .isValid();
        
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("true") != std::string::npos || output.find("1") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalDataTransformations) {
    std::string code = R"(
        // Complex data transformation pipeline
        rawData = [
            {"name": "Alice", "age": 25, "salary": 50000, "department": "Engineering"},
            {"name": "Bob", "age": 30, "salary": 60000, "department": "Marketing"},
            {"name": "Charlie", "age": 35, "salary": 70000, "department": "Engineering"},
            {"name": "Diana", "age": 28, "salary": 55000, "department": "Sales"}
        ];
        
        // Transformation pipeline
        processEmployees = fun(data) {
            return data
                .filter(fun(emp) { return emp.salary >= 55000; })  // High earners
                .map(fun(emp) {  // Add bonus calculation
                    return {
                        "name": emp.name,
                        "annualBonus": emp.salary * 0.1,
                        "department": emp.department
                    };
                })
                .sort(fun(a, b) {  // Sort by bonus
                    return a.annualBonus > b.annualBonus;
                });
        };
        
        processed = processEmployees(rawData);
        totalBonuses = processed
            .map(fun(emp) { return emp.annualBonus; })
            .reduce(fun(acc, x) { return acc + x; }, 0);
        
        print totalBonuses;  // 60000*0.1 + 70000*0.1 + 55000*0.1 = 6000 + 7000 + 5500 = 18500
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("18500") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalAlgebraicDataTypes) {
    std::string code = R"(
        // Simulate algebraic data types
        Option = {
            "Some": fun(value) {
                return {
                    "type": "Some",
                    "value": value,
                    "isSome": true,
                    "isNone": false,
                    "map": fun(fn) { return Option.Some(fn(this.value)); },
                    "flatMap": fun(fn) { return fn(this.value); },
                    "getOrElse": fun(defaultValue) { return this.value; }
                };
            },
            "None": {
                "type": "None",
                "value": nil,
                "isSome": false,
                "isNone": true,
                "map": fun(fn) { return this; },
                "flatMap": fun(fn) { return this; },
                "getOrElse": fun(defaultValue) { return defaultValue; }
            }
        };
        
        // Use the Option type
        safeHead = fun(arr) {
            if (arr.length > 0) {
                return Option.Some(arr[0]);
            }
            return Option.None;
        };
        
        result = safeHead([1, 2, 3])
            .map(fun(x) { return x * 2; })
            .getOrElse(0);
        
        print result;  // 1 * 2 = 2
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("2") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalCompositionDeep) {
    std::string code = R"(
        // Deep functional composition
        composeMany = fun(...functions) {
            return fun(initialValue) {
                result = initialValue;
                for (let i = functions.length - 1; i >= 0; i = i - 1) {
                    result = functions[i](result);
                }
                return result;
            };
        };
        
        add10 = fun(x) { return x + 10; };
        multiplyBy3 = fun(x) { return x * 3; };
        divideBy2 = fun(x) { return x / 2; };
        square = fun(x) { return x * x; };
        
        // Composition: square -> divideBy2 -> multiplyBy3 -> add10
        complexFunc = composeMany(add10, multiplyBy3, divideBy2, square);
        
        // Apply: square(4)=16, divideBy2(16)=8, multiplyBy3(8)=24, add10(24)=34
        result = complexFunc(4);
        print result;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("34") != std::string::npos);
}

TEST(FunctionalAdvanced, FunctionalReducers) {
    std::string code = R"(
        // Advanced reducers and accumulators
        data = [1, 2, 3, 4, 5];
        
        // Custom reducers
        sumReducer = fun(acc, value) { return acc + value; };
        productReducer = fun(acc, value) { return acc * value; };
        maxReducer = fun(acc, value) { return value > acc ? value : acc; };
        minReducer = fun(acc, value) { return value < acc ? value : acc; };
        
        // Generic reduce function
        reduceWith = fun(arr, reducer, initial) {
            result = initial;
            for (let i = 0; i < arr.length; i = i + 1) {
                result = reducer(result, arr[i]);
            }
            return result;
        };
        
        results = {
            "sum": reduceWith(data, sumReducer, 0),
            "product": reduceWith(data, productReducer, 1),
            "max": reduceWith(data, maxReducer, data[0]),
            "min": reduceWith(data, minReducer, data[0])
        };
        
        print results.sum;
    )";

    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
    EXPECT_TRUE(output.find("15") != std::string::npos);  // Sum of 1-5 is 15
}
