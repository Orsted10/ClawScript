#include "gc_alloc.h"
namespace volt {
std::shared_ptr<VoltArray> gcNewArray() {
    auto a = gcAcquireArrayFromPool();
    if (a) return a;
    return std::make_shared<VoltArray>();
}
std::shared_ptr<VoltArray> gcNewArray(const std::vector<Value>& elements) {
    auto a = gcAcquireArrayFromPool();
    if (a) {
        a->fill(nilValue(), 0);
        a->reserve(elements.size());
        for (const auto& e : elements) a->push(e);
        return a;
    }
    return std::make_shared<VoltArray>(elements);
}
std::shared_ptr<VoltArray> gcNewArrayReserved(size_t reserve) {
    auto a = gcAcquireArrayFromPool();
    if (!a) a = std::make_shared<VoltArray>();
    a->fill(nilValue(), 0);
    a->reserve(reserve);
    return a;
}
std::shared_ptr<VoltArray> gcNewArrayFilled(size_t n, Value v) {
    auto a = gcAcquireArrayFromPool();
    if (!a) return std::make_shared<VoltArray>(std::vector<Value>(n, v));
    a->fill(v, n);
    return a;
}
std::shared_ptr<VoltHashMap> gcNewHashMap() {
    auto m = gcAcquireHashMapFromPool();
    if (m) return m;
    return std::make_shared<VoltHashMap>();
}
std::shared_ptr<VoltInstance> gcNewInstance(std::shared_ptr<VoltClass> cls) { return std::make_shared<VoltInstance>(std::move(cls)); }
}
