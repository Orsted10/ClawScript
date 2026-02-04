// Example: JSON Handling in VoltScript
print("=== JSON Handling Demo ===");

// Create a complex data structure
person = {
    "name": "John Doe",
    "age": 35,
    "email": "john@example.com",
    "address": {
        "street": "123 Main St",
        "city": "Anytown",
        "zip": "12345"
    },
    "hobbies": ["reading", "swimming", "coding"],
    "active": true
};

// Encode to JSON
jsonString = jsonEncode(person);
print("Encoded JSON:");
print(jsonString);

// Decode from JSON
decodedPerson = jsonDecode(jsonString);
print("\nDecoded person name: " + decodedPerson.name);
print("Decoded person age: " + str(decodedPerson.age));
print("Address city: " + decodedPerson.address.city);
print("First hobby: " + decodedPerson.hobbies[0]);

// Working with arrays of objects
people = [
    { "name": "Alice", "age": 28, "department": "Engineering" },
    { "name": "Bob", "age": 32, "department": "Marketing" },
    { "name": "Carol", "age": 45, "department": "Sales" }
];

// Encode the array
peopleJson = jsonEncode(people);
print("\nPeople array as JSON:");
print(peopleJson);

// Decode and process
decodedPeople = jsonDecode(peopleJson);
print("\nProcessing people:");
for (let i = 0; i < decodedPeople.length; i = i + 1) {
    person = decodedPeople[i];
    print("  " + person.name + " is " + str(person.age) + " years old, works in " + person.department);
}

// Modify data and re-encode
print("\nModifying data...");
people[0].salary = 75000;
people[1].salary = 65000;
people[2].salary = 70000;

updatedJson = jsonEncode(people);
print("Updated JSON with salaries:");
print(updatedJson);

// Practical example: Configuration file simulation
config = {
    "appName": "MyVoltApp",
    "version": "1.0.0",
    "settings": {
        "debug": true,
        "maxUsers": 100,
        "timeout": 30,
        "features": {
            "logging": true,
            "analytics": false,
            "notifications": true
        }
    },
    "database": {
        "host": "localhost",
        "port": 5432,
        "name": "mydb"
    }
};

configJson = jsonEncode(config);
print("\nApplication configuration as JSON:");
print(configJson);

// Decode and use configuration
appConfig = jsonDecode(configJson);
print("\nUsing configuration:");
print("App: " + appConfig.appName + " v" + appConfig.version);
print("Debug mode: " + str(appConfig.settings.debug));
print("Max users: " + str(appConfig.settings.maxUsers));
print("DB host: " + appConfig.database.host + ":" + str(appConfig.database.port));

// Data transformation example
rawData = "[{\"id\":1,\"value\":\"A\"},{\"id\":2,\"value\":\"B\"},{\"id\":3,\"value\":\"C\"}]";
parsedData = jsonDecode(rawData);

print("\nTransforming data:");
transformed = [];
for (let i = 0; i < parsedData.length; i = i + 1) {
    item = {
        "id": parsedData[i].id,
        "value": parsedData[i].value,
        "computed": parsedData[i].id * 10
    };
    transformed.push(item);
}

resultJson = jsonEncode(transformed);
print("Transformed data:");
print(resultJson);