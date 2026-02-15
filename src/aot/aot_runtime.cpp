#include "llvm_aot.h"
#include "vm/vm.h"
#include "features/string_pool.h"
#include <cstring>

extern "C" int volt_aot_run(const uint8_t* code, uint64_t codeSize, const volt::AotConstant* consts, uint64_t constCount) {
    using namespace volt;

    Chunk chunk;
    for (uint64_t i = 0; i < codeSize; i++) {
        chunk.write(code[i], 0);
    }

    for (uint64_t i = 0; i < constCount; i++) {
        const auto& entry = consts[i];
        if (entry.tag == static_cast<uint8_t>(AotConstTag::Nil)) {
            chunk.addConstant(nilValue());
        } else if (entry.tag == static_cast<uint8_t>(AotConstTag::Bool)) {
            chunk.addConstant(boolValue(entry.payload != 0));
        } else if (entry.tag == static_cast<uint8_t>(AotConstTag::Number)) {
            double num;
            uint64_t bits = entry.payload;
            std::memcpy(&num, &bits, sizeof(double));
            chunk.addConstant(numberToValue(num));
        } else if (entry.tag == static_cast<uint8_t>(AotConstTag::String)) {
            const char* text = reinterpret_cast<const char*>(entry.payload);
            auto sv = StringPool::intern(text ? text : "");
            chunk.addConstant(stringValue(sv.data()));
        } else {
            chunk.addConstant(nilValue());
        }
    }

    VM vm;
    auto result = vm.interpret(chunk);
    return result == InterpretResult::Ok ? 0 : 1;
}
