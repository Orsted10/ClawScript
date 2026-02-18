#include "gc_alloc.h"
#include "observability/profiler.h"
namespace volt {
std::shared_ptr<VoltArray> gcNewArray() {
    auto a = gcAcquireArrayFromPool();
    if (a) return a;
    auto p = std::make_shared<VoltArray>();
    profilerRecordAlloc(sizeof(VoltArray), "array");
    return p;
}
std::shared_ptr<VoltArray> gcNewArray(const std::vector<Value>& elements) {
    auto a = gcAcquireArrayFromPool();
    if (a) {
        a->fill(nilValue(), 0);
        a->reserve(elements.size());
        for (const auto& e : elements) a->push(e);
        profilerRecordAlloc(sizeof(VoltArray) + elements.size() * sizeof(Value), "array");
        return a;
    }
    auto p = std::make_shared<VoltArray>(elements);
    profilerRecordAlloc(sizeof(VoltArray) + elements.size() * sizeof(Value), "array");
    return p;
}
std::shared_ptr<VoltArray> gcNewArrayReserved(size_t reserve) {
    auto a = gcAcquireArrayFromPool();
    if (!a) {
        a = std::make_shared<VoltArray>();
        profilerRecordAlloc(sizeof(VoltArray) + reserve * sizeof(Value), "array");
    } else {
        profilerRecordAlloc(sizeof(VoltArray) + reserve * sizeof(Value), "array");
    }
    a->fill(nilValue(), 0);
    a->reserve(reserve);
    return a;
}
std::shared_ptr<VoltArray> gcNewArrayFilled(size_t n, Value v) {
    auto a = gcAcquireArrayFromPool();
    if (!a) {
        auto p = std::make_shared<VoltArray>(std::vector<Value>(n, v));
        profilerRecordAlloc(sizeof(VoltArray) + n * sizeof(Value), "array");
        return p;
    }
    a->fill(v, n);
    profilerRecordAlloc(sizeof(VoltArray) + n * sizeof(Value), "array");
    return a;
}
std::shared_ptr<VoltHashMap> gcNewHashMap() {
    auto m = gcAcquireHashMapFromPool();
    if (m) return m;
    auto p = std::make_shared<VoltHashMap>();
    profilerRecordAlloc(sizeof(VoltHashMap), "hashmap");
    return p;
}
std::shared_ptr<VoltInstance> gcNewInstance(std::shared_ptr<VoltClass> cls) {
    auto p = std::make_shared<VoltInstance>(std::move(cls));
    profilerRecordAlloc(sizeof(VoltInstance), "instance");
    return p;
}
}
