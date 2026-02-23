#pragma once
#include <memory>

namespace claw {
class Environment;
class Interpreter;

void registerNativeArray(const std::shared_ptr<Environment>& globals, Interpreter& interpreter);
} // namespace claw
