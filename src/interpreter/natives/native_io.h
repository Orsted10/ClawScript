#pragma once
#include <memory>

namespace volt {
class Environment;

void registerNativeIO(const std::shared_ptr<Environment>& globals);
} // namespace volt
