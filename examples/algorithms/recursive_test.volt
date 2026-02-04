obj = { "name": "self" };
obj.ref = obj;  // Self-reference

// Access through the reference
nameThroughRef = obj.ref.name;
print nameThroughRef;