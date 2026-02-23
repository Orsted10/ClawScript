#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "vm/vm.h"
#include <sstream>
#include <iostream>

class PrintCapture {
public:
    PrintCapture() : old(std::cout.rdbuf(buffer.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(old); }
    std::string get() { return buffer.str(); }
private:
    std::stringstream buffer;
    std::streambuf* old;
};
static std::string runCodeIC(const std::string& source) {
    PrintCapture capture;
    claw::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    claw::Parser parser(tokens);
    auto statements = parser.parseProgram();
    if (parser.hadError()) return "PARSE_ERROR";
    claw::Interpreter interpreter;
    try {
        interpreter.execute(statements);
        return capture.get();
    } catch (...) {
        return "RUNTIME_ERROR";
    }
}

TEST(CallICDiagnostics, CrashReproductionDisabledIC) {
    claw::gRuntimeFlags.disableCallIC = true;
    claw::gRuntimeFlags.icDiagnostics = true;
    std::string src =
        "fn add(a, b) { return a + b; }"
        "let i = 0;"
        "while (i < 1000) {"
        "  let r = add(1, 2);"
        "  if (r != 3) { print \"wrong\"; }"
        "  i = i + 1;"
        "}"
        "print \"done\";";
    std::string out = runCodeIC(src);
    EXPECT_NE(out, "RUNTIME_ERROR");
    EXPECT_NE(out, "PARSE_ERROR");
    EXPECT_TRUE(out.find("done") != std::string::npos);
    claw::gRuntimeFlags.disableCallIC = false;
    claw::gRuntimeFlags.icDiagnostics = false;
}
TEST(CallICDiagnostics, EnabledICNoCrash) {
    claw::gRuntimeFlags.disableCallIC = false;
    claw::gRuntimeFlags.icDiagnostics = true;
    std::string src =
        "fn add(a, b) { return a + b; }"
        "let i = 0;"
        "while (i < 1000) {"
        "  let r = add(1, 2);"
        "  if (r != 3) { print \"wrong\"; }"
        "  i = i + 1;"
        "}"
        "print \"done\";";
    std::string out = runCodeIC(src);
    EXPECT_NE(out, "RUNTIME_ERROR");
    EXPECT_NE(out, "PARSE_ERROR");
    EXPECT_TRUE(out.find("done") != std::string::npos);
    claw::gRuntimeFlags.icDiagnostics = false;
}
