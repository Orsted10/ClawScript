#pragma once
#include <memory>
#include "interpreter/environment.h"
#include "interpreter/interpreter.h"
namespace claw {
void registerNativeSecurity(const std::shared_ptr<Environment>& globals, Interpreter& I);
}
