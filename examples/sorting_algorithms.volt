// Example: Sorting Algorithms in VoltScript
print("=== Sorting Algorithms Demo ===");

// Bubble Sort Implementation
bubbleSort = fun(arr) {
    n = arr.length;
    for (let i = 0; i < n; i = i + 1) {
        for (let j = 0; j < n - i - 1; j = j + 1) {
            if (arr[j] > arr[j + 1]) {
                // Swap elements
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    return arr;
};

// Selection Sort Implementation
selectionSort = fun(arr) {
    n = arr.length;
    for (let i = 0; i < n; i = i + 1) {
        minIdx = i;
        for (let j = i + 1; j < n; j = j + 1) {
            if (arr[j] < arr[minIdx]) {
                minIdx = j;
            }
        }
        // Swap elements
        temp = arr[minIdx];
        arr[minIdx] = arr[i];
        arr[i] = temp;
    }
    return arr;
};

// Insertion Sort Implementation
insertionSort = fun(arr) {
    for (let i = 1; i < arr.length; i = i + 1) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
    return arr;
};

// Generate random array for testing
generateRandomArray = fun(size, minVal, maxVal) {
    arr = [];
    for (let i = 0; i < size; i = i + 1) {
        val = floor(random() * (maxVal - minVal + 1)) + minVal;
        arr.push(val);
    }
    return arr;
};

// Print array helper
printArray = fun(arr, label) {
    print(label + ": [");
    for (let i = 0; i < arr.length; i = i + 1) {
        if (i == arr.length - 1) {
            print("  " + str(arr[i]));
        } else {
            print("  " + str(arr[i]) + ",");
        }
    }
    print("]");
};

// Test with different arrays
originalArray = [64, 34, 25, 12, 22, 11, 90];
print("Original array:");
printArray(originalArray, "");

// Test bubble sort
bubbleSorted = [];
for (let i = 0; i < originalArray.length; i = i + 1) {
    bubbleSorted.push(originalArray[i]);
}
bubbleResult = bubbleSort(bubbleSorted);
print("\nBubble sorted:");
printArray(bubbleResult, "");

// Test selection sort
selectionSorted = [];
for (let i = 0; i < originalArray.length; i = i + 1) {
    selectionSorted.push(originalArray[i]);
}
selectionResult = selectionSort(selectionSorted);
print("\nSelection sorted:");
printArray(selectionResult, "");

// Test insertion sort
insertionSorted = [];
for (let i = 0; i < originalArray.length; i = i + 1) {
    insertionSorted.push(originalArray[i]);
}
insertionResult = insertionSort(insertionSorted);
print("\nInsertion sorted:");
printArray(insertionResult, "");

// Performance comparison
testArray = generateRandomArray(10, 1, 100);
print("\nPerformance comparison on random array:");
printArray(testArray, "Test array");

// Benchmark each algorithm
bubbleBench = benchmark(bubbleSort, [testArray[0], testArray[1], testArray[2], testArray[3], testArray[4]]);
selectionBench = benchmark(selectionSort, [testArray[0], testArray[1], testArray[2], testArray[3], testArray[4]]);
insertionBench = benchmark(insertionSort, [testArray[0], testArray[1], testArray[2], testArray[3], testArray[4]]);

print("\nPerformance comparison (small array):");
print("Bubble sort: " + str(bubbleBench.timeMicroseconds) + " μs");
print("Selection sort: " + str(selectionBench.timeMicroseconds) + " μs");
print("Insertion sort: " + str(insertionBench.timeMicroseconds) + " μs");

// Test with larger array
largerArray = generateRandomArray(20, 1, 100);
print("\nTesting with larger array (20 elements):");

// Just test insertion sort on larger array as it's typically fastest for smaller datasets
largerCopy = [];
for (let i = 0; i < largerArray.length; i = i + 1) {
    largerCopy.push(largerArray[i]);
}

largerBench = benchmark(insertionSort, largerCopy);
print("Insertion sort on 20 elements: " + str(largerBench.timeMicroseconds) + " μs");
printArray(largerBench.result, "Sorted result");

// String sorting
fruitNames = ["banana", "apple", "orange", "grape", "kiwi"];
print("\nSorting strings:");
printArray(fruitNames, "Original");

// Simple string sort using insertion sort approach
stringSort = fun(arr) {
    for (let i = 1; i < arr.length; i = i + 1) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
    return arr;
};

sortedFruits = stringSort(fruitNames);
printArray(sortedFruits, "Sorted");

// Reverse sort simulation (sort then reverse)
numbersToReverse = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
print("\nReverse sorting (sort then reverse):");
printArray(numbersToReverse, "Original");

// Sort first
sortedDesc = [10, 9, 8, 7, 6, 5, 4, 3, 2, 1];  // Manually creating descending array
printArray(sortedDesc, "Manually created descending");