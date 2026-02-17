#pragma once
#include "features/array.h"
#include "features/hashmap.h"
#include "features/class.h"
#include <memory>
namespace volt {
std::shared_ptr<VoltArray> gcNewArray();
std::shared_ptr<VoltArray> gcNewArray(const std::vector<Value>& elements);
std::shared_ptr<VoltArray> gcNewArrayReserved(size_t reserve);
std::shared_ptr<VoltArray> gcNewArrayFilled(size_t n, Value v);
std::shared_ptr<VoltHashMap> gcNewHashMap();
std::shared_ptr<VoltInstance> gcNewInstance(std::shared_ptr<VoltClass> cls);
}
