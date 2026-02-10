#pragma once
#include <memory>

namespace volt {
class Environment;

void registerNativeTime(const std::shared_ptr<Environment>& globals);
} // namespace volt
