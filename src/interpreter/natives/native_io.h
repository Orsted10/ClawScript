#pragma once
#include <memory>

namespace claw {
class Environment;

void registerNativeIO(const std::shared_ptr<Environment>& globals);
} // namespace claw
