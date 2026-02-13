#pragma once
#include <string>
#include <unordered_set>
#include <string_view>
#include <mutex>

namespace volt {

/**
 * @brief Thread-safe String Pool for string interning
 * 
 * This ensures that identical strings share the same memory location,
 * allowing for fast string comparisons (pointer comparison instead of content).
 */
class StringPool {
public:
    static StringPool& getInstance() {
        static StringPool instance;
        return instance;
    }

    // Intern a string and return a stable string_view to it
    static std::string_view intern(const std::string& str) {
        return getInstance().internImpl(str);
    }
    static std::string_view intern(std::string_view str) {
        return getInstance().internImpl(str);
    }

    // Statistics
    size_t size() const { return pool_.size(); }
    void clear();

private:
    StringPool() = default;
    ~StringPool() = default;
    StringPool(const StringPool&) = delete;
    StringPool& operator=(const StringPool&) = delete;

    std::string_view internImpl(const std::string& str);
    std::string_view internImpl(std::string_view str);

    std::unordered_set<std::string> pool_;
    mutable std::mutex mutex_;
};

} // namespace volt
