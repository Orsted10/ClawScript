#include "string_pool.h"

namespace claw {

std::string_view StringPool::internImpl(const std::string& str) {
    {
        std::shared_lock<std::shared_mutex> rlock(mutex_);
        auto it = pool_.find(str);
        if (it != pool_.end()) return *it;
    }
    {
        std::unique_lock<std::shared_mutex> wlock(mutex_);
        auto it = pool_.find(str);
        if (it != pool_.end()) return *it;
        auto [inserted_it, inserted] = pool_.insert(str);
        (void)inserted;
        return *inserted_it;
    }
}

std::string_view StringPool::internImpl(std::string_view str) {
    {
        std::shared_lock<std::shared_mutex> rlock(mutex_);
        auto it = pool_.find(str);
        if (it != pool_.end()) return *it;
    }
    {
        std::unique_lock<std::shared_mutex> wlock(mutex_);
        auto it = pool_.find(str);
        if (it != pool_.end()) return *it;
        auto [inserted_it, inserted] = pool_.insert(std::string(str));
        (void)inserted;
        return *inserted_it;
    }
}

void StringPool::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    pool_.clear();
}

} // namespace claw
