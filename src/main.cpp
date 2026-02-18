#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "ast.h"
#include "stmt.h"
#include "token.h"
#include "version.h"
#include "compiler/compiler.h"
#ifdef VOLT_ENABLE_AOT
#include "aot/llvm_aot.h"
#endif
#include "vm/vm.h"
#include "jit/jit.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include "observability/profiler.h"

/******  FOR UTF-8 In Window Terminal or Powershell. ***********/
#ifdef _WIN32
    #include <windows.h>
#endif

/***************************************************************/

// Helper to print tokens for debug mode
void dumpTokens(const std::vector<volt::Token>& tokens) {
    std::cout << "\n=== TOKENS ===\n";
    for (const auto& tok : tokens) {
        std::cout << "[" << tok.line << ":" << tok.column << "] "
                  << volt::tokenName(tok.type) << " '" << tok.lexeme << "'";
        if (tok.type == volt::TokenType::String && !tok.stringValue.empty()) {
            std::cout << " -> \"" << tok.stringValue << "\"";
        }
        std::cout << "\n";
    }
    std::cout << "==============\n\n";
}

// Helper to print statements for debug mode
void dumpStatements(const std::vector<volt::StmtPtr>& statements) {
    std::cout << "\n=== AST ===\n";
    for (size_t i = 0; i < statements.size(); i++) {
        std::cout << i + 1 << ": ";
        if (auto* exprStmt = dynamic_cast<volt::ExprStmt*>(statements[i].get())) {
            std::cout << "ExprStmt: " << volt::printAST(exprStmt->expr.get());
        } else if (auto* printStmt = dynamic_cast<volt::PrintStmt*>(statements[i].get())) {
            std::cout << "PrintStmt: " << volt::printAST(printStmt->expr.get());
        } else if (auto* letStmt = dynamic_cast<volt::LetStmt*>(statements[i].get())) {
            std::cout << "LetStmt: " << letStmt->name;
            if (letStmt->initializer) {
                std::cout << " = " << volt::printAST(letStmt->initializer.get());
            }
        } else if (dynamic_cast<volt::IfStmt*>(statements[i].get())) {
            std::cout << "IfStmt";
        } else if (dynamic_cast<volt::WhileStmt*>(statements[i].get())) {
            std::cout << "WhileStmt";
        } else if (dynamic_cast<volt::ForStmt*>(statements[i].get())) {
            std::cout << "ForStmt";
        } else if (auto* fnStmt = dynamic_cast<volt::FnStmt*>(statements[i].get())) {
            std::cout << "FnStmt: " << fnStmt->name << "(";
            for (size_t j = 0; j < fnStmt->parameters.size(); j++) {
                if (j > 0) std::cout << ", ";
                std::cout << fnStmt->parameters[j];
            }
            std::cout << ")";
        } else if (dynamic_cast<volt::ReturnStmt*>(statements[i].get())) {
            std::cout << "ReturnStmt";
        } else if (dynamic_cast<volt::BreakStmt*>(statements[i].get())) {
            std::cout << "BreakStmt";
        } else if (dynamic_cast<volt::ContinueStmt*>(statements[i].get())) {
            std::cout << "ContinueStmt";
        } else if (dynamic_cast<volt::BlockStmt*>(statements[i].get())) {
            std::cout << "BlockStmt";
        } else {
            std::cout << "Unknown";
        }
        std::cout << "\n";
    }
    std::cout << "===========\n\n";
}

void printRuntimeError(const volt::RuntimeError& e) {
    std::cerr << "❌ " << volt::errorCodeToString(e.code) << ": Runtime Error [Line " << e.token.line 
              << ", Col " << e.token.column << "]: " 
              << e.what() << "\n";
    
    if (!e.stack_trace.empty()) {
        std::cerr << "Stack trace:\n";
        for (auto it = e.stack_trace.rbegin(); it != e.stack_trace.rend(); ++it) {
            std::cerr << "  at " << it->function_name << " (";
            if (!it->file_path.empty()) {
                std::cerr << it->file_path << ":";
            }
            std::cerr << it->line << ")\n";
        }
    }
}

void runFile(const std::string& path, volt::Interpreter& interpreter, bool debugMode) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << "\n";
        exit(74);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    // Tokenize
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    // Debug: print tokens
    if (debugMode) {
        dumpTokens(tokens);
    }
    
    // Parse
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) {
        for (const auto& error : parser.getErrors()) {
            std::cerr << error << "\n";
        }
        exit(65);
    }
    
    // Debug: print AST
    if (debugMode) {
        dumpStatements(statements);
    }
    
    // Execute
    try {
        interpreter.execute(statements);
    } catch (const volt::RuntimeError& e) {
        printRuntimeError(e);
        exit(70);
    }
}

bool compileFileToChunk(const std::string& path, std::unique_ptr<volt::Chunk>& chunk, bool debugMode) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    if (debugMode) {
        dumpTokens(tokens);
    }

    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();

    if (parser.hadError()) {
        for (const auto& error : parser.getErrors()) {
            std::cerr << error << "\n";
        }
        return false;
    }

    if (debugMode) {
        dumpStatements(statements);
    }

    volt::Compiler compiler;
    chunk = compiler.compile(statements);
    return true;
}

// Check if input is incomplete (unbalanced braces/parens/strings)
bool isIncomplete(const std::string& input) {
    int braces = 0, parens = 0;
    bool inString = false;
    
    for (size_t i = 0; i < input.size(); i++) {
        char c = input[i];
        
        // Handle string state (with escape handling)
        if (c == '"' && (i == 0 || input[i - 1] != '\\')) {
            inString = !inString;
            continue;
        }
        
        if (!inString) {
            if (c == '{') braces++;
            else if (c == '}') braces--;
            else if (c == '(') parens++;
            else if (c == ')') parens--;
        }
    }
    
    return braces > 0 || parens > 0 || inString;
}

void runPrompt() {
    volt::Interpreter interpreter;
    std::vector<std::string> history;
    std::string buffer;
    
    std::cout << "\n⚡ VoltScript v" << volt::VOLT_VERSION << " REPL\n";
    std::cout << "Type 'exit' to quit, 'history' to show command history\n";
    std::cout << "Commands: clear (reset environment), help (show this message)\n\n";
    
    while (true) {
        // Shows different prompt for continuation lines
        std::cout << (buffer.empty() ? "> " : ". ");
        
        std::string line;
        if (!std::getline(std::cin, line)) break;
        
        // Handle special commands only on fresh input
        if (buffer.empty()) {
            if (line == "exit" || line == "quit") break;
            if (line == "history") {
                std::cout << "--- Command History ---\n";
                for (size_t i = 0; i < history.size(); i++) {
                    std::cout << i + 1 << ": " << history[i] << "\n";
                }
                std::cout << "-----------------------\n";
                continue;
            }
            if (line == "clear") {
                interpreter.reset();
                std::cout << "Environment cleared.\n";
                continue;
            }
            if (line == "help") {
                std::cout << "\n=== VoltScript v" << volt::VOLT_VERSION << " Help ===\n";
                std::cout << "Special commands:\n";
                std::cout << "  exit/quit    - Exit the REPL\n";
                std::cout << "  history      - Show command history\n";
                std::cout << "  clear        - Reset environment\n";
                std::cout << "  help         - Show this help\n\n";
                std::cout << "Features:\n";
                std::cout << "  • Arrays with methods: push, pop, reverse, length\n";
                std::cout << "  • Hash maps with keys, values, has, remove\n";
                std::cout << "  • File I/O: readFile, writeFile, exists, fileSize\n";
                std::cout << "  • String functions: len, substr, indexOf, toUpper, toLower\n";
                std::cout << "  • Math functions: sin, cos, tan, log, exp, sqrt, pow\n";
                std::cout << "  • JSON support: jsonEncode, jsonDecode\n";
                std::cout << "  • Functional utilities: compose, pipe\n";
                std::cout << "  • Performance tools: sleep, benchmark\n\n";
                continue;
            }
            if (line.empty()) continue;
        }
        
        // Accumulate input
        buffer += (buffer.empty() ? "" : "\n") + line;
        
        // Check if input is complete
        if (isIncomplete(buffer)) {
            continue;  // Wait for more input
        }
        
        // Add to history before processing
        if (!buffer.empty()) {
            history.push_back(buffer);
        }
        
        try {
            // Tokenize
            volt::Lexer lexer(buffer);
            auto tokens = lexer.tokenize();
            
            // Parse
            volt::Parser parser(tokens);
            auto statements = parser.parseProgram();
            
            if (parser.hadError()) {
                for (const auto& error : parser.getErrors()) {
                    std::cerr << "❌ " << error << "\n";
                }
                buffer.clear();
                continue;
            }
            
            // Execute
            interpreter.execute(statements);
            
        } catch (const volt::RuntimeError& e) {
            printRuntimeError(e);
        } catch (const std::exception& e) {
            std::cerr << "❌ Error: " << e.what() << "\n";
        }
        
        buffer.clear();
    }
}

int main(int argc, char** argv) {

    // Handling UTF-8 in window Terminal/powershell
     #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif
    
    bool debugMode = false;
    std::string scriptPath;
    std::string aotOutputPath;
    bool jitAggressive = false;
    bool disableCallIC = false;
    bool icDiagnostics = false;
    bool enableProfile = false;
    std::string profileOutput;
    int profileHz = 100;
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            debugMode = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "⚡ VoltScript v" << volt::VOLT_VERSION << "\n";
            std::cout << "Usage: volt [options] [script]\n\n";
            std::cout << "Options:\n";
            std::cout << "  --debug, -d    Print tokens and AST before execution\n";
            std::cout << "  --help, -h     Show this help message\n";
            std::cout << "  --version      Show version information\n";
            std::cout << "  --aot-output   Emit LLVM AOT object file\n";
            std::cout << "  --jit=aggressive   Enable aggressive adaptive JIT\n";
            std::cout << "  --disable-call-ic   Disable interpreter call inline cache\n";
            std::cout << "  --ic-diagnostics    Enable call IC diagnostics logging\n";
            std::cout << "  --profile[=file]    Enable sampling + heap profiler and write HTML\n";
            std::cout << "  --profile-hz=NUM    Sampling frequency in Hz (default 100)\n";
            return 0;
        } else if (arg == "--version") {
            std::cout << "VoltScript " << volt::VOLT_VERSION << "\n";
            return 0;
        } else if (arg == "--jit=aggressive") {
            jitAggressive = true;
        } else if (arg == "--disable-call-ic") {
            disableCallIC = true;
        } else if (arg == "--ic-diagnostics") {
            icDiagnostics = true;
        } else if (arg.rfind("--aot-output=", 0) == 0) {
            aotOutputPath = arg.substr(std::string("--aot-output=").size());
        } else if (arg == "--aot-output") {
            if (i + 1 >= argc) {
                std::cerr << "--aot-output requires a path\n";
                return 64;
            }
            aotOutputPath = argv[++i];
        } else if (arg.rfind("--profile=", 0) == 0) {
            enableProfile = true;
            profileOutput = arg.substr(std::string("--profile=").size());
        } else if (arg == "--profile") {
            enableProfile = true;
        } else if (arg.rfind("--profile-hz=", 0) == 0) {
            try { profileHz = std::stoi(arg.substr(std::string("--profile-hz=").size())); } catch (...) {}
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            return 64;
        } else {
            if (!scriptPath.empty()) {
                std::cerr << "Only one script file can be specified\n";
                return 64;
            }
            scriptPath = arg;
        }
    }
    if (jitAggressive) {
        volt::gJitConfig.aggressive = true;
        volt::gJitConfig.loopThreshold = 1000;
        volt::gJitConfig.functionThreshold = 1000;
    }
    const char* envDisable = std::getenv("VOLT_DISABLE_CALL_IC");
    const char* envDiag = std::getenv("VOLT_IC_DIAGNOSTICS");
    const char* envProfile = std::getenv("VOLT_PROFILE");
    const char* envProfileHz = std::getenv("VOLT_PROFILE_HZ");
    const char* envProfileOut = std::getenv("VOLT_PROFILE_OUT");
    if (envDisable && *envDisable) disableCallIC = true;
    if (envDiag && *envDiag) icDiagnostics = true;
    if (!enableProfile && envProfile && *envProfile) enableProfile = true;
    if (envProfileHz && *envProfileHz) { try { profileHz = std::stoi(envProfileHz); } catch (...) {} }
    if (profileOutput.empty() && envProfileOut && *envProfileOut) profileOutput = envProfileOut;
    volt::gRuntimeFlags.disableCallIC = disableCallIC;
    volt::gRuntimeFlags.icDiagnostics = icDiagnostics;
    
    if (!aotOutputPath.empty()) {
        if (scriptPath.empty()) {
            std::cerr << "--aot-output requires a script file\n";
            return 64;
        }

#ifdef VOLT_ENABLE_AOT
        std::unique_ptr<volt::Chunk> chunk;
        if (!compileFileToChunk(scriptPath, chunk, debugMode)) {
            return 65;
        }

        volt::AotCompiler aotCompiler;
        auto module = aotCompiler.compile("volt_aot", *chunk);

        std::ofstream out(aotOutputPath, std::ios::binary);
        out.write(reinterpret_cast<const char*>(module.image.data()), static_cast<std::streamsize>(module.image.size()));
        if (!out) {
            std::cerr << "Failed to write AOT object: " << aotOutputPath << "\n";
            return 74;
        }

        std::filesystem::path objPath(aotOutputPath);
        std::filesystem::path exePath = objPath;
        exePath.replace_extension();
#ifdef _WIN32
        exePath.replace_extension(".exe");
        std::filesystem::path runtimeLib = std::filesystem::path(argv[0]).parent_path() / "volt_runtime.lib";
        std::string cmd = "lld-link /OUT:" + exePath.string() + " " + objPath.string() + " " + runtimeLib.string();
#else
        std::filesystem::path runtimeLib = std::filesystem::path(argv[0]).parent_path() / "libvolt_runtime.a";
        std::string cmd = "ld -o " + exePath.string() + " " + objPath.string() + " " + runtimeLib.string() + " -lstdc++ -lm -lc -lpthread";
#endif

        int linkResult = std::system(cmd.c_str());
        if (linkResult != 0) {
            std::cerr << "Linker failed with exit code " << linkResult << "\n";
            return linkResult;
        }
        return 0;
#else
        std::cerr << "AOT is not enabled in this build\n";
        return 64;
#endif
    }

    volt::Interpreter interpreter;
    if (enableProfile) {
        volt::profilerSetCurrentInterpreter(&interpreter);
        volt::profilerStart(profileHz);
    }
    
    if (!scriptPath.empty()) {
        // Run file
        runFile(scriptPath, interpreter, debugMode);
    } else {
        // Interactive REPL
        runPrompt();
    }
    if (enableProfile || volt::profilerEnabled()) {
        volt::profilerStop();
        volt::Profiler::instance().writeHtml(profileOutput);
        volt::Profiler::instance().writeSpeedscope(profileOutput);
    }
    
    return 0;
}
