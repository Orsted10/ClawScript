#pragma once
#include <memory>

namespace volt {
class Environment;

void registerNativeJSON(const std::shared_ptr<Environment>& globals);
} // namespace volt
