#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "value.h"
#include <mutex>
#include "observability/profiler.h"

namespace claw {

struct ClawHashMap;

using HashMapPtr = std::shared_ptr<ClawHashMap>;

/**
 * @brief Hash map/dictionary implementation for VoltScript
 * 
 * Stores key-value pairs where keys are strings and values can be any VoltScript type
 */
struct ClawHashMap {
    std::unordered_map<std::string, Value> data;
    size_t lastBuckets = 0;
    mutable std::mutex mu;
    
    // Constructor
    ClawHashMap() = default;
    
    // Copy constructor
    ClawHashMap(const std::unordered_map<std::string, Value>& initialData) : data(initialData) {}
    
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
        return claw::nilValue(); // Return nil if key doesn't exist
    }
    
    // Set key-value pair
    void set(const std::string& key, const Value& value) {
        gcBarrierWrite(this, value);
        size_t oldBuckets = data.bucket_count();
        data[key] = value;
        size_t newBuckets = data.bucket_count();
        if (newBuckets > oldBuckets) {
            size_t deltaBuckets = newBuckets - oldBuckets;
            profilerRecordAlloc(deltaBuckets * sizeof(void*) * 8, "hashmap.bucket.grow");
            lastBuckets = newBuckets;
        }
    }
    
    // Ensure key exists with a default value (thread-safe)
    void ensureDefault(const std::string& key, const Value& defaultValue) {
        std::lock_guard<std::mutex> lock(mu);
        if (data.find(key) == data.end()) {
            gcBarrierWrite(this, defaultValue);
            size_t oldBuckets = data.bucket_count();
            data[key] = defaultValue;
            size_t newBuckets = data.bucket_count();
            if (newBuckets > oldBuckets) {
                size_t deltaBuckets = newBuckets - oldBuckets;
                profilerRecordAlloc(deltaBuckets * sizeof(void*) * 8, "hashmap.bucket.grow");
                lastBuckets = newBuckets;
            }
        }
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
    bool operator==(const ClawHashMap& other) const {
        return data == other.data;
    }
    
    // Merge another hash map into this one
    void merge(const ClawHashMap& other) {
        for (const auto& [key, value] : other.data) {
            data[key] = value;
        }
    }
};

} // namespace claw
