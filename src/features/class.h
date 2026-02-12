#pragma once
#include "value.h"
#include "callable.h"
#include "../parser/stmt.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

namespace volt {

class VoltInstance;

/**
 * Represents a class in VoltScript
 */
class VoltClass : public Callable, public std::enable_shared_from_this<VoltClass> {
public:
    VoltClass(std::string name, std::shared_ptr<VoltClass> superclass, 
              std::unordered_map<std::string, std::shared_ptr<VoltFunction>> methods)
        : name_(std::move(name)), superclass_(std::move(superclass)), methods_(std::move(methods)) {}

    const std::string& getName() const { return name_; }
    std::shared_ptr<VoltClass> getSuperclass() const { return superclass_; }
    
    std::shared_ptr<VoltFunction> findMethod(const std::string& name) const;

    // Callable interface (creating an instance)
    Value call(Interpreter& interpreter, const std::vector<Value>& arguments) override;
    int arity() const override;
    std::string toString() const override { return "<class " + name_ + ">"; }

private:
    std::string name_;
    std::shared_ptr<VoltClass> superclass_;
    std::unordered_map<std::string, std::shared_ptr<VoltFunction>> methods_;
};

/**
 * Represents an instance of a class
 */
class VoltInstance : public std::enable_shared_from_this<VoltInstance> {
public:
    explicit VoltInstance(std::shared_ptr<VoltClass> cls) : class_(std::move(cls)) {}

    Value get(const Token& name);
    void set(const Token& name, Value value);

    std::string toString() const { return "<" + class_->getName() + " instance>"; }
    std::shared_ptr<VoltClass> getClass() const { return class_; }

private:
    std::shared_ptr<VoltClass> class_;
    std::unordered_map<std::string, Value> fields_;
};

} // namespace volt
