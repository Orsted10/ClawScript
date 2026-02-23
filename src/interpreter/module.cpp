#include "module.h"
#include "interpreter.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace claw {

std::shared_ptr<Module> ModuleManager::loadModule(const std::string& path, Interpreter& interpreter) {
    // 1. Resolve absolute path
    std::string absolutePath;
    try {
        absolutePath = std::filesystem::absolute(path).string();
    } catch (...) {
        absolutePath = path;
    }

    // 2. Check cache
    auto it = cache_.find(absolutePath);
    if (it != cache_.end()) {
        return it->second;
    }

    // 3. Read module source
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open module file: " + path);
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // 4. Tokenize and Parse
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();

    if (parser.hadError()) {
        std::string errors;
        for (const auto& err : parser.getErrors()) {
            errors += err + "\n";
        }
        throw std::runtime_error("Error parsing module " + path + ":\n" + errors);
    }

    // 5. Execute in a fresh environment (with globals)
    auto moduleEnv = std::make_shared<Environment>(interpreter.getGlobals());
    
    // Save current environment and switch to module environment
    auto previousEnv = interpreter.getEnvironment();
    
    try {
        interpreter.executeBlock(statements, moduleEnv);
    } catch (const std::exception& e) {
        throw std::runtime_error("Error executing module " + path + ": " + e.what());
    }

    // 6. Create and cache module
    auto module = std::make_shared<Module>(absolutePath, moduleEnv, std::move(statements));
    cache_[absolutePath] = module;

    return module;
}

} // namespace claw
