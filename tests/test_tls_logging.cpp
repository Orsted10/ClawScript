#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <sstream>
using namespace claw;
namespace {
class PrintCapture {
public:
    PrintCapture() : oldBuf_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(oldBuf_); }
    std::string getOutput() const { return buffer_.str(); }
private:
    std::stringstream buffer_;
    std::streambuf* oldBuf_;
};
std::string runCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    if (parser.hadError()) {
        return "PARSE_ERROR";
    }
    PrintCapture capture;
    Interpreter interpreter;
    try {
        interpreter.execute(statements);
    } catch (const std::exception& e) {
        return std::string("RUNTIME_ERROR: ") + e.what();
    }
    return capture.getOutput();
}
} // namespace
TEST(TLSPrimitives, GatingDisabled) {
    std::string out = runCode(R"(
        print(tlsGet("https://example.com/"));
    )");
    ASSERT_EQ(out, "RUNTIME_ERROR: Network disabled by sandbox");
}
TEST(TLSPrimitives, NetworkAllowedNoGatingError) {
    std::string out = runCode(R"(
        writeFile(".voltsec", "network=allow");
        policyReload();
        print(tlsGet("https://example.com/"));
    )");
    ASSERT_NE(out, "RUNTIME_ERROR: Network disabled by sandbox");
}
TEST(LogWrite, MetadataRequiredPolicy) {
    std::string out = runCode(R"(
        writeFile(".voltsec", "log.meta.required=true");
        policyReload();
        logWrite("no-meta");
        print("done");
    )");
    ASSERT_EQ(out, "RUNTIME_ERROR: Log metadata required by policy");
}
TEST(LogWrite, HmacAndMetadataWritten) {
    std::string out = runCode(R"(
        writeFile(".voltsec", "log.path=test_log.txt\nlog.hmac=abc123\noutput=allow");
        policyReload();
        logWrite("hello", {"a":1});
        print(readFile("test_log.txt"));
    )");
    ASSERT_NE(out, "PARSE_ERROR");
    ASSERT_NE(out, "");
    ASSERT_NE(out.find("hello|"), std::string::npos);
    ASSERT_NE(out.find("{"), std::string::npos);
    ASSERT_NE(out.find("}"), std::string::npos);
}

TEST(Sanity, MinimalPass) {
    ASSERT_TRUE(true);
}
