#include "string_pool.h"

namespace volt {

std::string_view StringPool::internImpl(const std::string& str) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pool_.find(str);
    if (it != pool_.end()) {
        return *it;
    }
    auto [inserted_it, inserted] = pool_.insert(str);
    return *inserted_it;
}

std::string_view StringPool::internImpl(std::string_view str) {
    std::lock_guard<std::mutex> lock(mutex_);
    // find with string_view is only available in C++20 with transparent hash/equal
    // For now, we convert to string to find
    std::string s(str);
    auto it = pool_.find(s);
    if (it != pool_.end()) {
        return *it;
    }
    auto [inserted_it, inserted] = pool_.insert(std::move(s));
    return *inserted_it;
}

void StringPool::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.clear();
}

} // namespace volt
