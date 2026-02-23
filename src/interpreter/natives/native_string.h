#pragma once
#include <memory>

namespace claw {
class Environment;

void registerNativeString(const std::shared_ptr<Environment>& globals);
} // namespace claw
