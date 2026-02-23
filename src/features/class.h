#pragma once
#include "value.h"
#include "callable.h"
#include "../parser/stmt.h"
#include <unordered_map>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include "features/string_pool.h"
#include <functional>

namespace claw {

class ClawInstance;

/**
 * Represents a class in VoltScript
 */
class ClawClass : public Callable, public std::enable_shared_from_this<ClawClass> {
public:
    ClawClass(std::string name, std::shared_ptr<ClawClass> superclass, 
              std::unordered_map<std::string, std::shared_ptr<ClawFunction>> methods)
        : name_(std::move(name)), superclass_(std::move(superclass)), methods_(std::move(methods)) {}

    const std::string& getName() const { return name_; }
    std::shared_ptr<ClawClass> getSuperclass() const { return superclass_; }
    
    std::shared_ptr<ClawFunction> findMethod(const std::string& name) const;

    // Callable interface (creating an instance)
    Value call(Interpreter& interpreter, const std::vector<Value>& arguments) override;
    int arity() const override;
    std::string toString() const override { return "<class " + name_ + ">"; }

private:
    std::string name_;
    std::shared_ptr<ClawClass> superclass_;
    std::unordered_map<std::string, std::shared_ptr<ClawFunction>> methods_;
};

/**
 * Represents an instance of a class
 */
class ClawInstance : public std::enable_shared_from_this<ClawInstance> {
public:
    explicit ClawInstance(std::shared_ptr<ClawClass> cls) : class_(std::move(cls)) {}

    Value get(const Token& name);
    void set(const Token& name, Value value);
    void forEachField(const std::function<void(Value)>& fn) const;
    bool has(const Token& name) const;

    std::string toString() const { return "<" + class_->getName() + " instance>"; }
    std::shared_ptr<ClawClass> getClass() const { return class_; }

private:
    std::shared_ptr<ClawClass> class_;
    struct InternedStringHash {
        size_t operator()(std::string_view sv) const {
            return std::hash<const char*>{}(sv.data());
        }
    };
    struct InternedStringEqual {
        bool operator()(std::string_view sv1, std::string_view sv2) const {
            return sv1.data() == sv2.data();
        }
    };
    std::unordered_map<std::string_view, Value, InternedStringHash, InternedStringEqual> fields_;
    std::unordered_map<std::string_view, Value, InternedStringHash, InternedStringEqual> ic_get_cache_;
};

} // namespace claw
