#pragma once
#include <string>
#include <unordered_set>
#include <string_view>
#include <shared_mutex>

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

    struct TransparentHash {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
        size_t operator()(const std::string& s) const noexcept { return std::hash<std::string_view>{}(s); }
    };
    struct TransparentEqual {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
        bool operator()(const std::string& a, const std::string& b) const noexcept { return a == b; }
        bool operator()(const std::string& a, std::string_view b) const noexcept { return a == b; }
        bool operator()(std::string_view a, const std::string& b) const noexcept { return a == b; }
    };

    std::unordered_set<std::string, TransparentHash, TransparentEqual> pool_;
    mutable std::shared_mutex mutex_;
};

} // namespace volt
