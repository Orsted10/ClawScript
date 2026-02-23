#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/parser.h"
#include "parser/ast.h"
#include <cstddef>
#include <cstdint>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    try {
        std::string src(reinterpret_cast<const char*>(data), size);
        claw::Lexer lex(src);
        auto tokens = lex.tokenize();
        claw::Parser parser(tokens);
        auto program = parser.parseProgram();
        // Walk a few statements to exercise AST creation
        for (const auto& st : program) {
            (void)st.get();
        }
    } catch (...) {
        // Swallow exceptions to let fuzzer continue
    }
    return 0;
}
