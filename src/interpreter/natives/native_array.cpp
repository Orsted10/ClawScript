#include "interpreter/natives/native_array.h"
#include "interpreter/environment.h"
#include "interpreter/interpreter.h"
#include "features/callable.h"
#include "features/array.h"
#include "interpreter/value.h"

namespace volt {

void registerNativeArray(const std::shared_ptr<Environment>& globals, Interpreter& interpreter) {
    globals->define("reverse", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isArray(args[0])) {
                throw std::runtime_error("reverse() requires an array argument");
            }
            auto original = asArray(args[0]);
            auto reversed = std::make_shared<VoltArray>();
            for (int i = original->length() - 1; i >= 0; i--) {
                reversed->push(original->get(i));
            }
            return reversed;
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
            auto result = std::make_shared<VoltArray>();
            for (size_t i = 0; i < array->size(); i++) {
                Value element = array->get(i);
                Value predicateResult = func->call(interpreter, {element});
                if (isTruthy(predicateResult)) {
                    result->push(element);
                }
            }
            return result;
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
            auto result = std::make_shared<VoltArray>();
            for (size_t i = 0; i < array->size(); i++) {
                Value element = array->get(i);
                Value mappedValue = func->call(interpreter, {element});
                result->push(mappedValue);
            }
            return result;
        },
        "map"
    ));
}

} // namespace volt
