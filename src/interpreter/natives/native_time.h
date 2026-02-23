#pragma once
#include <memory>

namespace claw {
class Environment;

void registerNativeTime(const std::shared_ptr<Environment>& globals);
} // namespace claw
