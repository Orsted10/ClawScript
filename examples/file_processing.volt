// File processing and I/O operations demonstration
print "=== File Processing Demo ===";

// Check if a file exists
let filename = "test_data.txt";

if (exists(filename)) {
    print "File " + filename + " exists";
    print "File size: " + str(fileSize(filename)) + " bytes";
} else {
    print "File " + filename + " does not exist, creating it...";
    
    // Create a file with sample data
    let content = "Line 1: Hello World\n";
    content = content + "Line 2: This is VoltScript\n";
    content = content + "Line 3: File processing demo\n";
    content = content + "Line 4: Numbers: 42, 3.14, 100\n";
    content = content + "Line 5: End of file\n";
    
    let writeResult = writeFile(filename, content);
    if (writeResult) {
        print "File created successfully";
    } else {
        print "Failed to create file";
    }
}

// Read the file
print "\n=== Reading File Content ===";
let fileContent = readFile(filename);
if (fileContent != nil) {
    print "File content:";
    print fileContent;
} else {
    print "Failed to read file";
}

// Process file content line by line
print "\n=== Line Processing ===";
let lines = split(fileContent, "\n");
let lineCount = 0;
let wordCount = 0;

for (let i = 0; i < lines.length; i = i + 1) {
    if (len(lines[i]) > 0) {
        lineCount = lineCount + 1;
        print "Line " + str(i + 1) + ": " + lines[i];
        
        // Simple word counting (count spaces + 1)
        let line = lines[i];
        let wordsInLine = 1;
        for (let j = 0; j < len(line); j = j + 1) {
            if (line[j] == " ") {
                wordsInLine = wordsInLine + 1;
            }
        }
        wordCount = wordCount + wordsInLine;
    }
}

print "\n=== File Statistics ===";
print "Total lines: " + str(lineCount);
print "Total words: " + str(wordCount);

// Search for specific content
print "\n=== Content Search ===";
let searchTerm = "VoltScript";
let found = false;
for (let i = 0; i < lines.length; i = i + 1) {
    if (indexOf(lines[i], searchTerm) != -1) {
        print "Found '" + searchTerm + "' in line " + str(i + 1) + ": " + lines[i];
        found = true;
    }
}

if (!found) {
    print "Search term '" + searchTerm + "' not found";
}

// Append to file
print "\n=== Appending to File ===";
let appendContent = "Appended line: Process completed at " + str(now()) + "\n";
let appendResult = appendFile(filename, appendContent);
if (appendResult) {
    print "Content appended successfully";
} else {
    print "Failed to append content";
}

// Read updated file
let updatedContent = readFile(filename);
print "\n=== Updated File Content ===";
print updatedContent;

print "\n=== File Processing Complete ===";