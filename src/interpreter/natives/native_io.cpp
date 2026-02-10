#include "interpreter/natives/native_io.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "interpreter/value.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace volt {

void registerNativeIO(const std::shared_ptr<Environment>& globals) {
    globals->define("input", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isString(args[0])) {
                std::cout << asString(args[0]);
            }
            std::string line;
            std::getline(std::cin, line);
            return line;
        },
        "input"
    ));

    globals->define("readFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("readFile() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            if (!file) {
                throw std::runtime_error("Could not open file: " + asString(args[0]));
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        },
        "readFile"
    ));

    globals->define("writeFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("writeFile() requires string path and content");
            }
            std::ofstream file(asString(args[0]));
            if (!file) {
                throw std::runtime_error("Could not open file for writing: " + asString(args[0]));
            }
            file << asString(args[1]);
            return true;
        },
        "writeFile"
    ));

    globals->define("appendFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("appendFile() requires string path and content");
            }
            std::ofstream file(asString(args[0]), std::ios::app);
            if (!file) {
                throw std::runtime_error("Could not open file for appending: " + asString(args[0]));
            }
            file << asString(args[1]);
            return true;
        },
        "appendFile"
    ));

    globals->define("fileExists", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("fileExists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return file.good();
        },
        "fileExists"
    ));

    globals->define("exists", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("exists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return file.good();
        },
        "exists"
    ));

    globals->define("deleteFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("deleteFile() requires a string path");
            }
            std::string path = asString(args[0]);
            if (std::remove(path.c_str()) == 0) {
                return true;
            } else {
                return false;
            }
        },
        "deleteFile"
    ));

    globals->define("fileSize", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("fileSize() requires a string path");
            }
            std::string path = asString(args[0]);
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) {
                throw std::runtime_error("Could not open file: " + path);
            }
            return static_cast<double>(file.tellg());
        },
        "fileSize"
    ));
}

} // namespace volt
