#include <benchmark/benchmark.h>
#include "features/string_pool.h"
#include <string>
#include <vector>
#include <random>

using namespace claw;

static void BM_StringInterning(benchmark::State& state) {
    std::vector<std::string> strings;
    for (int i = 0; i < 1000; ++i) {
        strings.push_back("string_" + std::to_string(i));
    }

    for (auto _ : state) {
        for (const auto& s : strings) {
            benchmark::DoNotOptimize(StringPool::intern(s));
        }
    }
}
BENCHMARK(BM_StringInterning);

static void BM_StringPoolLookup(benchmark::State& state) {
    std::string s = "test_string_for_lookup";
    StringPool::intern(s);

    for (auto _ : state) {
        benchmark::DoNotOptimize(StringPool::intern(s));
    }
}
BENCHMARK(BM_StringPoolLookup);

static void BM_StringComparison_Raw(benchmark::State& state) {
    std::string s1 = "a_very_long_string_to_compare_for_performance_reasons_1234567890";
    std::string s2 = "a_very_long_string_to_compare_for_performance_reasons_1234567890";
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(s1 == s2);
    }
}
BENCHMARK(BM_StringComparison_Raw);

static void BM_StringComparison_Interned(benchmark::State& state) {
    std::string_view s1 = StringPool::intern("a_very_long_string_to_compare_for_performance_reasons_1234567890");
    std::string_view s2 = StringPool::intern("a_very_long_string_to_compare_for_performance_reasons_1234567890");
    
    for (auto _ : state) {
        // For interned strings, we can just compare pointers
        benchmark::DoNotOptimize(s1.data() == s2.data());
    }
}
BENCHMARK(BM_StringComparison_Interned);
