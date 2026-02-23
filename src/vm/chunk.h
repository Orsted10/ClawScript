#pragma once
#include <vector>
#include <cstdint>
#include "opcodes.h"
#include "interpreter/value.h"

namespace claw {

/**
 * @brief A sequence of bytecode instructions and constants
 */
class Chunk {
public:
    Chunk() = default;

    // Write a byte to the chunk
    void write(uint8_t byte, int line) {
        code_.push_back(byte);
        lines_.push_back(line);
    }

    // Write an opcode to the chunk
    void write(OpCode opcode, int line) {
        write(static_cast<uint8_t>(opcode), line);
    }

    // Add a constant and return its index
    int addConstant(Value value) {
        constants_.push_back(value);
        return static_cast<int>(constants_.size() - 1);
    }

    // Accessors
    const std::vector<uint8_t>& code() const { return code_; }
    const std::vector<Value>& constants() const { return constants_; }
    int getLine(int offset) const { return lines_[offset]; }
    size_t size() const { return code_.size(); }
    int countOpcode(OpCode op) const {
        int c = 0;
        for (auto b : code_) {
            if (b == static_cast<uint8_t>(op)) c++;
        }
        return c;
    }
    void setLoopCount(int c) { loopCount_ = c; }
    int loopCount() const { return loopCount_; }

private:
    std::vector<uint8_t> code_;
    std::vector<int> lines_; // For error reporting
    std::vector<Value> constants_;
    int loopCount_ = 0;
};

} // namespace claw
