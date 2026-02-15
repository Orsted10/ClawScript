#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "value.h"

namespace volt {

// Forward declaration
struct VoltHashMap;

// Shared pointer type for hash maps
using HashMapPtr = std::shared_ptr<VoltHashMap>;

/**
 * @brief Hash map/dictionary implementation for VoltScript
 * 
 * Stores key-value pairs where keys are strings and values can be any VoltScript type
 */
struct VoltHashMap {
    std::unordered_map<std::string, Value> data;
    
    // Constructor
    VoltHashMap() = default;
    
    // Copy constructor
    VoltHashMap(const std::unordered_map<std::string, Value>& initialData) : data(initialData) {}
    
    // Get the number of key-value pairs
    size_t size() const { return data.size(); }
    
    // Check if the hash map is empty
    bool empty() const { return data.empty(); }
    
    // Check if a key exists
    bool contains(const std::string& key) const {
        return data.find(key) != data.end();
    }
    
    // Get value by key (returns nullptr if not found)
    Value get(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return it->second;
        }
        return volt::nilValue(); // Return nil if key doesn't exist
    }
    
    // Set key-value pair
    void set(const std::string& key, const Value& value) {
        data[key] = value;
    }
    
    // Remove a key-value pair
    bool remove(const std::string& key) {
        return data.erase(key) > 0;
    }
    
    // Get all keys as a vector (optimized)
    std::vector<std::string> getKeys() const {
        std::vector<std::string> keys;
        keys.reserve(data.size());
        keys.resize(data.size());
        size_t i = 0;
        for (const auto& pair : data) {
            keys[i++] = pair.first;
        }
        return keys;
    }
    
    // Get all values as a vector (optimized)
    std::vector<Value> getValues() const {
        std::vector<Value> values;
        values.reserve(data.size());
        values.resize(data.size());
        size_t i = 0;
        for (const auto& pair : data) {
            values[i++] = pair.second;
        }
        return values;
    }
    
    // Clear all entries
    void clear() { data.clear(); }
    
    // Equality comparison
    bool operator==(const VoltHashMap& other) const {
        return data == other.data;
    }
    
    // Merge another hash map into this one
    void merge(const VoltHashMap& other) {
        for (const auto& [key, value] : other.data) {
            data[key] = value;
        }
    }
};

} // namespace volt
