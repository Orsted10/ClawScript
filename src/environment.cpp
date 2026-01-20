#include "environment.h"

namespace volt {

Environment::Environment(std::shared_ptr<Environment> parent)
    : enclosing(std::move(parent)) {}

void Environment::define(const std::string& name, Value value) {
    values[name] = value;
}

Value Environment::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end()) {
        return it->second;
    }

    if (enclosing) {
        return enclosing->get(name);
    }

    throw std::runtime_error(
        "Runtime Error: Undefined variable '" + name + "'"
    );
}

void Environment::assign(const std::string& name, Value value) {
    auto it = values.find(name);
    if (it != values.end()) {
        it->second = value;
        return;
    }

    if (enclosing) {
        enclosing->assign(name, value);
        return;
    }

    throw std::runtime_error(
        "Runtime Error: Undefined variable '" + name + "'"
    );
}

bool Environment::existsInCurrentScope(const std::string& name) const {
    return values.find(name) != values.end();
}

} // namespace volt
