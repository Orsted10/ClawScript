#pragma once
#include <memory>

namespace claw {
class Environment;

void registerNativeMath(const std::shared_ptr<Environment>& globals);

/**
 * Optimized math operations
 */
double fastPow(double base, int exp);
} // namespace claw
