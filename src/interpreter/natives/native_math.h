#pragma once
#include <memory>

namespace volt {
class Environment;

void registerNativeMath(const std::shared_ptr<Environment>& globals);
} // namespace volt
