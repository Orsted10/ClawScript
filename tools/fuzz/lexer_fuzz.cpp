#include "lexer/lexer.h"
#include "lexer/token.h"
#include <cstddef>
#include <cstdint>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    try {
        std::string src(reinterpret_cast<const char*>(data), size);
        claw::Lexer lex(src);
        auto tokens = lex.tokenize();
        // Touch a few fields to exercise code paths
        for (const auto& t : tokens) {
            (void)t.type;
            (void)t.line;
            (void)t.column;
            if (t.lexeme.size() > 1024) break;
        }
    } catch (...) {
        // Swallow exceptions to let fuzzer continue
    }
    return 0;
}
