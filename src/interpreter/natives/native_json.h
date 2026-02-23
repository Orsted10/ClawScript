#pragma once
#include <memory>

namespace claw {
class Environment;

void registerNativeJSON(const std::shared_ptr<Environment>& globals);
} // namespace claw
