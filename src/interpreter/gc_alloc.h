#pragma once
#include "features/array.h"
#include "features/hashmap.h"
#include "features/class.h"
#include <memory>
namespace claw {
std::shared_ptr<ClawArray> gcNewArray();
std::shared_ptr<ClawArray> gcNewArray(const std::vector<Value>& elements);
std::shared_ptr<ClawArray> gcNewArrayReserved(size_t reserve);
std::shared_ptr<ClawArray> gcNewArrayFilled(size_t n, Value v);
std::shared_ptr<ClawHashMap> gcNewHashMap();
std::shared_ptr<ClawInstance> gcNewInstance(std::shared_ptr<ClawClass> cls);
}
