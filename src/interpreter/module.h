#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "value.h"
#include "environment.h"

#include <vector>
#include "../parser/stmt.h"

namespace volt {

/**
 * Represents a loaded module
 */
class Module {
public:
    Module(std::string path, std::shared_ptr<Environment> env, std::vector<StmtPtr> statements)
        : path_(std::move(path)), environment_(std::move(env)), statements_(std::move(statements)) {}

    const std::string& getPath() const { return path_; }
    std::shared_ptr<Environment> getEnvironment() const { return environment_; }
    const std::vector<StmtPtr>& getStatements() const { return statements_; }

    // Get an exported value from the module
    Value getExport(const std::string& name) const {
        return environment_->get(name);
    }

private:
    std::string path_;
    std::shared_ptr<Environment> environment_;
    std::vector<StmtPtr> statements_;
};

/**
 * Manages module loading and caching
 */
class ModuleManager {
public:
    ModuleManager() = default;

    // Load a module from a file path
    // Returns cached module if already loaded
    std::shared_ptr<Module> loadModule(const std::string& path, class Interpreter& interpreter);

    // Clear module cache
    void clearCache() { cache_.clear(); }

private:
    // Resolve module path (check current dir and modules/ dir)
    std::string resolvePath(const std::string& path, const std::string& currentScriptDir);

    std::unordered_map<std::string, std::shared_ptr<Module>> cache_;
};

} // namespace volt
