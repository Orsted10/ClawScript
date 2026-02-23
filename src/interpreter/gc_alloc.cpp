#include "gc_alloc.h"
#include "observability/profiler.h"
namespace claw {
std::shared_ptr<ClawArray> gcNewArray() {
    auto a = gcAcquireArrayFromPool();
    if (a) return a;
    auto p = std::make_shared<ClawArray>();
    profilerRecordAlloc(sizeof(ClawArray), "array");
    return p;
}
std::shared_ptr<ClawArray> gcNewArray(const std::vector<Value>& elements) {
    auto a = gcAcquireArrayFromPool();
    if (a) {
        a->fill(nilValue(), 0);
        a->reserve(elements.size());
        for (const auto& e : elements) a->push(e);
        profilerRecordAlloc(sizeof(ClawArray) + elements.size() * sizeof(Value), "array");
        return a;
    }
    auto p = std::make_shared<ClawArray>(elements);
    profilerRecordAlloc(sizeof(ClawArray) + elements.size() * sizeof(Value), "array");
    return p;
}
std::shared_ptr<ClawArray> gcNewArrayReserved(size_t reserve) {
    auto a = gcAcquireArrayFromPool();
    if (!a) {
        a = std::make_shared<ClawArray>();
        profilerRecordAlloc(sizeof(ClawArray) + reserve * sizeof(Value), "array");
    } else {
        profilerRecordAlloc(sizeof(ClawArray) + reserve * sizeof(Value), "array");
    }
    a->fill(nilValue(), 0);
    a->reserve(reserve);
    return a;
}
std::shared_ptr<ClawArray> gcNewArrayFilled(size_t n, Value v) {
    auto a = gcAcquireArrayFromPool();
    if (!a) {
        auto p = std::make_shared<ClawArray>(std::vector<Value>(n, v));
        profilerRecordAlloc(sizeof(ClawArray) + n * sizeof(Value), "array");
        return p;
    }
    a->fill(v, n);
    profilerRecordAlloc(sizeof(ClawArray) + n * sizeof(Value), "array");
    return a;
}
std::shared_ptr<ClawHashMap> gcNewHashMap() {
    auto m = gcAcquireHashMapFromPool();
    if (m) return m;
    auto p = std::make_shared<ClawHashMap>();
    profilerRecordAlloc(sizeof(ClawHashMap), "hashmap");
    return p;
}
std::shared_ptr<ClawInstance> gcNewInstance(std::shared_ptr<ClawClass> cls) {
    auto p = std::make_shared<ClawInstance>(std::move(cls));
    profilerRecordAlloc(sizeof(ClawInstance), "instance");
    return p;
}
}
