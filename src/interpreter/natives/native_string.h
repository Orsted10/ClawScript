#pragma once
#include <memory>

namespace volt {
class Environment;

void registerNativeString(const std::shared_ptr<Environment>& globals);
} // namespace volt
