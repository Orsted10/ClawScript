#include "interpreter/natives/native_math.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace volt {

double fastPow(double base, int exp) {
    if (exp == 0) return 1.0;
    if (exp < 0) return 1.0 / fastPow(base, -exp);
    double res = 1.0;
    while (exp > 0) {
        if (exp % 2 == 1) res *= base;
        base *= base;
        exp /= 2;
    }
    return res;
}

void registerNativeMath(const std::shared_ptr<Environment>& globals) {
    globals->define("abs", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("abs() requires a number");
            return std::abs(asNumber(args[0]));
        },
        "abs"
    ));

    globals->define("sqrt", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("sqrt() requires a number");
            double val = asNumber(args[0]);
            if (val < 0) throw std::runtime_error("sqrt() argument must be non-negative");
            return std::sqrt(val);
        },
        "sqrt"
    ));

    globals->define("pow", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]) || !isNumber(args[1])) {
                throw std::runtime_error("pow() requires two numbers");
            }
            double base = asNumber(args[0]);
            double exp = asNumber(args[1]);
            
            // Optimization: Use fastPow for integer exponents
            if (std::floor(exp) == exp) {
                return fastPow(base, static_cast<int>(exp));
            }
            
            return std::pow(base, exp);
        },
        "pow"
    ));

    globals->define("min", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]) || !isNumber(args[1])) {
                throw std::runtime_error("min() requires two numbers");
            }
            return std::min(asNumber(args[0]), asNumber(args[1]));
        },
        "min"
    ));

    globals->define("max", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]) || !isNumber(args[1])) {
                throw std::runtime_error("max() requires two numbers");
            }
            return std::max(asNumber(args[0]), asNumber(args[1]));
        },
        "max"
    ));

    globals->define("round", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("round() requires a number");
            return std::round(asNumber(args[0]));
        },
        "round"
    ));

    globals->define("floor", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("floor() requires a number");
            return std::floor(asNumber(args[0]));
        },
        "floor"
    ));

    globals->define("ceil", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("ceil() requires a number");
            return std::ceil(asNumber(args[0]));
        },
        "ceil"
    ));

    globals->define("random", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            return static_cast<double>(std::rand()) / RAND_MAX;
        },
        "random"
    ));

    globals->define("sin", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("sin() requires a number");
            return std::sin(asNumber(args[0]));
        },
        "sin"
    ));

    globals->define("cos", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("cos() requires a number");
            return std::cos(asNumber(args[0]));
        },
        "cos"
    ));

    globals->define("tan", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("tan() requires a number");
            return std::tan(asNumber(args[0]));
        },
        "tan"
    ));

    globals->define("log", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("log() requires a number");
            double x = asNumber(args[0]);
            if (x <= 0) throw std::runtime_error("log() argument must be positive");
            return std::log(x);
        },
        "log"
    ));

    globals->define("exp", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("exp() requires a number");
            return std::exp(asNumber(args[0]));
        },
        "exp"
    ));
}

} // namespace volt
