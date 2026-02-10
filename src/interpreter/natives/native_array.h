#pragma once
#include <memory>

namespace volt {
class Environment;
class Interpreter;

void registerNativeArray(const std::shared_ptr<Environment>& globals, Interpreter& interpreter);
} // namespace volt
