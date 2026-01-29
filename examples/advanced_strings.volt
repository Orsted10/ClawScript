// Example: Advanced String Operations in VoltScript
print("=== Advanced String Operations Demo ===");

// padStart and padEnd
text = "Hello";
paddedLeft = padStart(text, 10, " ");
paddedRight = padEnd(text, 10, " ");
paddedZeros = padStart(text, 10, "0");

print("Original: '" + text + "'");
print("Padded left: '" + paddedLeft + "'");
print("Padded right: '" + paddedRight + "'");
print("Padded with zeros: '" + paddedZeros + "'");

// repeat
word = "Hi ";
repeated = repeat(word, 5);
print("Repeated: " + repeated);

// charCodeAt and fromCharCode
char = "A";
code = charCodeAt(char, 0);
print("Character code of '" + char + "': " + str(code));

charBack = fromCharCode(code);
print("Character from code " + str(code) + ": " + charBack);

// More character codes
alphabet = "";
for (let i = 65; i < 75; i = i + 1) {
    alphabet = alphabet + fromCharCode(i);
}
print("Alphabet from codes 65-74: " + alphabet);

// Practical example: Formatting numbers
number = 42;
formatted = padStart(str(number), 5, "0");
print("Formatted number: " + formatted);