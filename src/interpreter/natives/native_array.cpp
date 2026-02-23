#include "interpreter/natives/native_array.h"
#include "interpreter/environment.h"
#include "interpreter/interpreter.h"
#include "features/callable.h"
#include "features/array.h"
#include "interpreter/value.h"
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#endif

namespace claw {

void registerNativeArray(const std::shared_ptr<Environment>& globals, Interpreter& interpreter) {
    globals->define("reverse", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isArray(args[0])) {
                throw std::runtime_error("reverse() requires an array argument");
            }
            auto original = asArray(args[0]);
            auto reversed = std::make_shared<ClawArray>();
            for (int i = original->length() - 1; i >= 0; i--) {
                reversed->push(original->get(i));
            }
            return arrayValue(reversed);
        },
        "reverse"
    ));

    globals->define("filter", std::make_shared<NativeFunction>(
        2,
        [&interpreter](const std::vector<Value>& args) -> Value {
            if (!isArray(args[0])) {
                throw std::runtime_error("filter() requires an array as first argument");
            }
            if (!isCallable(args[1])) {
                throw std::runtime_error("filter() requires a function as second argument");
            }
            auto array = asArray(args[0]);
            auto func = asCallable(args[1]);
            auto result = std::make_shared<ClawArray>();
            for (size_t i = 0; i < array->size(); i++) {
                Value element = array->get(i);
                Value predicateResult = func->call(interpreter, {element});
                if (isTruthy(predicateResult)) {
                    result->push(element);
                }
            }
            return arrayValue(result);
        },
        "filter"
    ));

    globals->define("map", std::make_shared<NativeFunction>(
        2,
        [&interpreter](const std::vector<Value>& args) -> Value {
            if (!isArray(args[0])) {
                throw std::runtime_error("map() requires an array as first argument");
            }
            if (!isCallable(args[1])) {
                throw std::runtime_error("map() requires a function as second argument");
            }
            auto array = asArray(args[0]);
            auto func = asCallable(args[1]);
            auto result = std::make_shared<ClawArray>();
            for (size_t i = 0; i < array->size(); i++) {
                Value element = array->get(i);
                Value mappedValue = func->call(interpreter, {element});
                result->push(mappedValue);
            }
            return arrayValue(result);
        },
        "map"
    ));

    globals->define("map_add_scalar", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isArray(args[0])) {
                throw std::runtime_error("map_add_scalar() requires an array as first argument");
            }
            if (!isNumber(args[1])) {
                throw std::runtime_error("map_add_scalar() requires a number as second argument");
            }
            auto array = asArray(args[0]);
            double add = asNumber(args[1]);
            auto result = std::make_shared<ClawArray>();
            size_t n = array->size();
            size_t i = 0;
#if defined(__AVX__) || defined(__AVX2__)
            __m256d vadd = _mm256_set1_pd(add);
            for (; i + 4 <= n; i += 4) {
                double a0 = isNumber(array->get(i)) ? asNumber(array->get(i)) : 0.0;
                double a1 = isNumber(array->get(i + 1)) ? asNumber(array->get(i + 1)) : 0.0;
                double a2 = isNumber(array->get(i + 2)) ? asNumber(array->get(i + 2)) : 0.0;
                double a3 = isNumber(array->get(i + 3)) ? asNumber(array->get(i + 3)) : 0.0;
                __m256d v = _mm256_set_pd(a3, a2, a1, a0);
                __m256d r = _mm256_add_pd(vadd, v);
                alignas(32) double out[4];
                _mm256_store_pd(out, r);
                result->push(numberToValue(out[0]));
                result->push(numberToValue(out[1]));
                result->push(numberToValue(out[2]));
                result->push(numberToValue(out[3]));
            }
#endif
            for (; i < n; ++i) {
                Value el = array->get(i);
                if (isNumber(el)) {
                    result->push(numberToValue(asNumber(el) + add));
                } else {
                    result->push(el);
                }
            }
            return arrayValue(result);
        },
        "map_add_scalar"
    ));

    globals->define("array_sum", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isArray(args[0])) {
                throw std::runtime_error("array_sum() requires an array argument");
            }
            auto array = asArray(args[0]);
            size_t n = array->size();
            size_t i = 0;
            double sum = 0.0;
#if defined(__AVX__) || defined(__AVX2__)
            __m256d vacc = _mm256_set1_pd(0.0);
            for (; i + 4 <= n; i += 4) {
                double a0 = isNumber(array->get(i)) ? asNumber(array->get(i)) : 0.0;
                double a1 = isNumber(array->get(i + 1)) ? asNumber(array->get(i + 1)) : 0.0;
                double a2 = isNumber(array->get(i + 2)) ? asNumber(array->get(i + 2)) : 0.0;
                double a3 = isNumber(array->get(i + 3)) ? asNumber(array->get(i + 3)) : 0.0;
                __m256d v = _mm256_set_pd(a3, a2, a1, a0);
                vacc = _mm256_add_pd(vacc, v);
            }
            alignas(32) double buf[4];
            _mm256_store_pd(buf, vacc);
            sum += buf[0] + buf[1] + buf[2] + buf[3];
#endif
            for (; i < n; ++i) {
                Value el = array->get(i);
                if (isNumber(el)) sum += asNumber(el);
            }
            return numberToValue(sum);
        },
        "array_sum"
    ));
}

} // namespace claw
