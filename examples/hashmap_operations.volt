// Example: Hash Map Operations in VoltScript
print("=== Hash Map Operations Demo ===");

// Create a basic hash map
student = {
    "name": "John Smith",
    "age": 20,
    "grade": "A",
    "courses": ["Math", "Physics", "Computer Science"]
};

print("Student record:");
print("  Name: " + student.name);
print("  Age: " + str(student.age));
print("  Grade: " + student.grade);
print("  Courses: " + student.courses[0] + ", " + student.courses[1] + ", " + student.courses[2]);

// Access values and check existence
print("\nAccessing values:");
print("  Student name: " + student.name);
print("  Has grade: " + str(student.grade != nil));

// Working with multiple records
students = {
    "S001": {
        "name": "Alice Johnson",
        "major": "Computer Science",
        "year": 3
    },
    "S002": {
        "name": "Bob Williams",
        "major": "Mathematics",
        "year": 2
    },
    "S003": {
        "name": "Carol Brown",
        "major": "Physics",
        "year": 4
    }
};

print("\nStudents in the system:");
ids = keys(students);
for (let i = 0; i < ids.length; i = i + 1) {
    id = ids[i];
    student = students[id];
    print("  ID: " + id + ", Name: " + student.name + ", Major: " + student.major);
}

// Using hash map methods
print("\nUsing hash map methods:");
print("Number of students: " + str(students.size));

studentIds = students.keys();
print("Student IDs: [");
for (let i = 0; i < studentIds.length; i = i + 1) {
    if (i == studentIds.length - 1) {
        print("  " + studentIds[i]);
    } else {
        print("  " + studentIds[i] + ",");
    }
}
print("]");

// Check if a specific student exists
if (students.has("S001")) {
    print("Student S001 exists: " + students["S001"].name);
} else {
    print("Student S001 does not exist");
}

// Add a new student
students["S004"] = {
    "name": "David Davis",
    "major": "Chemistry",
    "year": 1
};
print("Added new student. Total now: " + str(students.size));

// Update existing record
students["S001"].year = 4;  // Alice is now in year 4
print("Updated Alice's year: " + str(students["S001"].year));

// Remove a student
wasRemoved = students.remove("S002");
print("Removed Bob (was removed: " + str(wasRemoved) + "). Total now: " + str(students.size));

// Working with configuration settings
config = {
    "server": {
        "host": "localhost",
        "port": 8080,
        "ssl": false
    },
    "database": {
        "name": "myapp_db",
        "user": "admin",
        "connections": 10
    },
    "features": {
        "logging": true,
        "caching": true,
        "compression": false
    }
};

print("\nConfiguration:");
print("Server: " + config.server.host + ":" + str(config.server.port));
print("Database: " + config.database.name);
print("Logging enabled: " + str(config.features.logging));

// Dynamic hash map operations
stats = {};
data = [1, 2, 2, 3, 3, 3, 4, 4, 4, 4];

// Count occurrences
for (let i = 0; i < data.length; i = i + 1) {
    value = str(data[i]);
    if (stats.has(value)) {
        stats[value] = stats[value] + 1;
    } else {
        stats[value] = 1;
    }
}

print("\nValue counts:");
statKeys = stats.keys();
for (let i = 0; i < statKeys.length; i = i + 1) {
    key = statKeys[i];
    print("  " + key + ": " + str(stats[key]) + " occurrences");
}

// Nested hash maps
company = {
    "name": "TechCorp",
    "departments": {
        "engineering": {
            "manager": "Sarah Lee",
            "employees": 25,
            "projects": ["Project A", "Project B"]
        },
        "marketing": {
            "manager": "Mike Johnson",
            "employees": 12,
            "campaigns": ["Summer Sale", "Holiday Promo"]
        },
        "hr": {
            "manager": "Lisa Chen",
            "employees": 5,
            "policies": ["Remote Work", "Health Insurance"]
        }
    }
};

print("\nCompany structure:");
depts = company.departments.keys();
for (let i = 0; i < depts.length; i = i + 1) {
    dept = depts[i];
    manager = company.departments[dept].manager;
    empCount = company.departments[dept].employees;
    print("  " + dept + ": Managed by " + manager + ", " + str(empCount) + " employees");
}

// Practical example: Shopping cart
cart = {
    "items": {},
    "total": 0,
    "currency": "USD"
};

addItem = fun(cart, name, price, quantity) {
    if (!cart.items.has(name)) {
        cart.items[name] = { "price": price, "quantity": quantity };
    } else {
        cart.items[name].quantity = cart.items[name].quantity + quantity;
    }
    cart.total = cart.total + (price * quantity);
    return cart;
};

// Add items to cart
addItem(cart, "Laptop", 999.99, 1);
addItem(cart, "Mouse", 25.50, 2);
addItem(cart, "Keyboard", 75.00, 1);

print("\nShopping cart:");
itemNames = cart.items.keys();
for (let i = 0; i < itemNames.length; i = i + 1) {
    item = itemNames[i];
    details = cart.items[item];
    subtotal = details.price * details.quantity;
    print("  " + item + ": $" + str(details.price) + " x " + str(details.quantity) + " = $" + str(subtotal));
}
print("Total: $" + str(cart.total) + " " + cart.currency);