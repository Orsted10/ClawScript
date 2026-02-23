#include "interpreter/natives/native_time.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include "features/string_pool.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <ctime>

namespace claw {

void registerNativeTime(const std::shared_ptr<Environment>& globals) {
    globals->define("clock", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return numberToValue(static_cast<double>(millis) / 1000.0);
        },
        "clock"
    ));

    globals->define("now", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return numberToValue(static_cast<double>(millis));
        },
        "now"
    ));

    globals->define("formatDate", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("formatDate() requires a timestamp number as first argument");
            if (!isString(args[1])) throw std::runtime_error("formatDate() requires a format string as second argument");
            double timestampMs = asNumber(args[0]);
            std::string format = asString(args[1]);
            auto secs = static_cast<long long>(timestampMs / 1000.0);
            std::time_t t = static_cast<std::time_t>(secs);
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &t);
#else
            tm = *std::localtime(&t);
#endif
            if (format.find('%') != std::string::npos) {
                char buf[128];
                std::strftime(buf, sizeof(buf), format.c_str(), &tm);
                auto sv = StringPool::intern(std::string(buf));
                return stringValue(sv.data());
            } else {
                auto pad2 = [](int v) {
                    std::ostringstream oss;
                    oss << std::setw(2) << std::setfill('0') << v;
                    return oss.str();
                };
                auto replaceAll = [](std::string s, const std::string& token, const std::string& repl) {
                    size_t pos = 0;
                    while ((pos = s.find(token, pos)) != std::string::npos) {
                        s.replace(pos, token.size(), repl);
                        pos += repl.size();
                    }
                    return s;
                };
                bool hasTokens = (format.find("YYYY") != std::string::npos);
                if (hasTokens) {
                    std::string out = format;
                    out = replaceAll(out, "YYYY", std::to_string(1900 + tm.tm_year));
                    out = replaceAll(out, "MM", pad2(1 + tm.tm_mon));
                    out = replaceAll(out, "DD", pad2(tm.tm_mday));
                    out = replaceAll(out, "HH", pad2(tm.tm_hour));
                    out = replaceAll(out, "mm", pad2(tm.tm_min));
                    out = replaceAll(out, "ss", pad2(tm.tm_sec));
                    auto sv = StringPool::intern(out);
                    return stringValue(sv.data());
                } else {
                    std::ostringstream oss;
                    oss << "Date(" << static_cast<long long>(timestampMs) << ") formatted as '" << format << "'";
                    auto sv = StringPool::intern(oss.str());
                    return stringValue(sv.data());
                }
            }
        },
        "formatDate"
    ));

    globals->define("sleep", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) {
                throw std::runtime_error("sleep() requires a number (milliseconds)");
            }
            int ms = static_cast<int>(asNumber(args[0]));
            if (ms < 0) ms = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            return boolValue(true);
        },
        "sleep"
    ));
}

} // namespace claw
