#ifndef VOLT_STACK_TRACE_H
#define VOLT_STACK_TRACE_H

#include <string>
#include <vector>
#include <iostream>
#include "errors.h"

namespace claw {

struct StackFrame {
    std::string function_name;
    int line;
    std::string file_path;

    StackFrame(std::string name, int l, std::string file = "")
        : function_name(std::move(name)), line(l), file_path(std::move(file)) {}
};

class CallStack {
public:
    static constexpr size_t MAX_DEPTH = 1000;

    void push(std::string function_name, int line, std::string file = "") {
        if (frames.size() >= MAX_DEPTH) {
            throw std::runtime_error("Stack overflow: Maximum call depth exceeded");
        }
        frames.emplace_back(std::move(function_name), line, std::move(file));
    }

    void pop() {
        if (!frames.empty()) {
            frames.pop_back();
        }
    }

    const std::vector<StackFrame>& get_frames() const {
        return frames;
    }

    bool empty() const {
        return frames.empty();
    }

    size_t size() const {
        return frames.size();
    }

    void clear() {
        frames.clear();
    }

private:
    std::vector<StackFrame> frames;
};

} // namespace claw

#endif // VOLT_STACK_TRACE_H
