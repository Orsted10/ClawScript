// JSON data processing and manipulation demonstration
print "=== JSON Data Processing ===";

// Create complex data structures
print "=== Creating Data Structures ===";

let userData = {
    "name": "John Doe",
    "age": 30,
    "active": true,
    "scores": [85, 92, 78, 96],
    "address": {
        "street": "123 Main St",
        "city": "New York",
        "zipcode": "10001"
    },
    "hobbies": ["reading", "swimming", "coding"],
    "metadata": {
        "created": now(),
        "version": "1.0"
    }
};

print "User data created successfully";

// JSON encoding and decoding
print "\n=== JSON Operations ===";

// Encode to JSON string
let jsonString = jsonEncode(userData);
print "JSON encoded string length: " + str(len(jsonString));

// Decode back to object
let decodedData = jsonDecode(jsonString);
print "Decoded data name: " + str(decodedData.name);
print "Decoded data age: " + str(decodedData.age);

// Verify data integrity
if (userData.name == decodedData.name && userData.age == decodedData.age) {
    print "Data integrity verified - encoding/decoding successful";
} else {
    print "Data integrity check failed";
}

// Data processing and analysis
print "\n=== Data Analysis ===";

// Calculate average score
let scores = userData.scores;
let totalScore = 0;
for (let i = 0; i < scores.length; i = i + 1) {
    totalScore = totalScore + scores[i];
}
let averageScore = totalScore / scores.length;
print "Average score: " + str(averageScore);

// Find highest score
let highestScore = scores[0];
for (let i = 1; i < scores.length; i = i + 1) {
    if (scores[i] > highestScore) {
        highestScore = scores[i];
    }
}
print "Highest score: " + str(highestScore);

// Count hobbies
print "Number of hobbies: " + str(userData.hobbies.length);
print "Hobbies: " + str(userData.hobbies);

// Nested data access
print "\n=== Nested Data Access ===";
print "Street: " + str(userData.address.street);
print "City: " + str(userData.address.city);
print "Zipcode: " + str(userData.address.zipcode);

// Data filtering and transformation
print "\n=== Data Transformation ===";

// Filter high scores (above 90)
let highScores = [];
for (let i = 0; i < scores.length; i = i + 1) {
    if (scores[i] >= 90) {
        highScores.push(scores[i]);
    }
}
print "High scores (90+): " + str(highScores);

// Transform hobbies to uppercase
let uppercaseHobbies = [];
for (let i = 0; i < userData.hobbies.length; i = i + 1) {
    uppercaseHobbies.push(toUpper(userData.hobbies[i]));
}
print "Uppercase hobbies: " + str(uppercaseHobbies);

// Create summary report
print "\n=== Summary Report ===";

let report = {
    "user": userData.name,
    "age": userData.age,
    "status": userData.active ? "Active" : "Inactive",
    "averageScore": averageScore,
    "totalScores": scores.length,
    "location": userData.address.city + ", " + userData.address.street,
    "primaryHobby": userData.hobbies[0],
    "reportGenerated": now()
};

let reportJson = jsonEncode(report);
print "Report generated successfully";
print "Report JSON length: " + str(len(reportJson));

// Save report to file
let saveResult = writeFile("user_report.json", reportJson);
if (saveResult) {
    print "Report saved to user_report.json";
} else {
    print "Failed to save report";
}

// Read and process the saved report
let savedReport = readFile("user_report.json");
if (savedReport != nil) {
    let parsedReport = jsonDecode(savedReport);
    print "=== Saved Report Details ===";
    print "User: " + str(parsedReport.user);
    print "Status: " + str(parsedReport.status);
    print "Average Score: " + str(parsedReport.averageScore);
    print "Location: " + str(parsedReport.location);
} else {
    print "Failed to read saved report";
}

// Multiple user data processing
print "\n=== Multiple Users Processing ===";

let users = [
    {
        "id": 1,
        "name": "Alice",
        "department": "Engineering",
        "salary": 75000,
        "skills": ["Python", "JavaScript", "C++"]
    },
    {
        "id": 2,
        "name": "Bob",
        "department": "Marketing",
        "salary": 65000,
        "skills": ["SEO", "Content Writing", "Analytics"]
    },
    {
        "id": 3,
        "name": "Charlie",
        "department": "Engineering",
        "salary": 80000,
        "skills": ["Java", "Spring", "Database Design"]
    }
];

// Process user data
let engineeringUsers = [];
let totalSalary = 0;

for (let i = 0; i < users.length; i = i + 1) {
    let user = users[i];
    totalSalary = totalSalary + user.salary;
    
    if (user.department == "Engineering") {
        engineeringUsers.push(user);
    }
}

let averageSalary = totalSalary / users.length;
print "Total salary: $" + str(totalSalary);
print "Average salary: $" + str(averageSalary);
print "Engineering users count: " + str(engineeringUsers.length);

// Export processed data
let processedData = {
    "users": users,
    "statistics": {
        "totalUsers": users.length,
        "averageSalary": averageSalary,
        "engineeringUsers": engineeringUsers.length,
        "departments": ["Engineering", "Marketing"]
    },
    "timestamp": now()
};

let processedJson = jsonEncode(processedData);
let exportResult = writeFile("processed_users.json", processedJson);
if (exportResult) {
    print "Processed data exported successfully";
} else {
    print "Failed to export processed data";
}

// Configuration data handling
print "\n=== Configuration Handling ===";

let config = {
    "database": {
        "host": "localhost",
        "port": 5432,
        "name": "voltscript_db",
        "ssl": true
    },
    "server": {
        "port": 8080,
        "timeout": 30000,
        "debug": false
    },
    "features": {
        "logging": true,
        "caching": true,
        "authentication": true
    }
};

print "Configuration loaded";
print "Database host: " + str(config.database.host);
print "Server port: " + str(config.server.port);
print "SSL enabled: " + str(config.database.ssl);

// Update configuration
config.server.debug = true;
config.features.newFeature = "beta";

let updatedConfigJson = jsonEncode(config);
print "Configuration updated and encoded";

print "\n=== JSON Data Processing Complete ===";