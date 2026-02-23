#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "interpreter/environment.h"
#include "ast.h"
#include "stmt.h"
#include "token.h"
#include "version.h"
#include "compiler/compiler.h"
#ifdef CLAW_ENABLE_AOT
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
#include <map>

/******  FOR UTF-8 In Window Terminal or Powershell. ***********/
#ifdef _WIN32
    #include <windows.h>
#endif

/***************************************************************/

static claw::Environment::SandboxMode g_cliSandboxMode = claw::Environment::SandboxMode::Full;
static std::map<std::string, std::string> g_policyKVs;

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static void loadVoltsecPolicy(const std::filesystem::path& baseDir) {
    std::filesystem::path p = baseDir / ".voltsec";
    g_policyKVs.clear();
    try {
        std::ifstream f(p);
        if (!f) return;
        std::string line;
        while (std::getline(f, line)) {
            auto t = trim(line);
            if (t.empty() || t[0] == '#') continue;
            auto eq = t.find('=');
            if (eq == std::string::npos) continue;
            std::string key = trim(t.substr(0, eq));
            std::string val = trim(t.substr(eq + 1));
            g_policyKVs[key] = val;
        }
    } catch (...) {}
}

static void applyPolicyToEnv(const std::shared_ptr<claw::Environment>& env) {
    auto sv = [](const std::string& k, const std::string& def) {
        auto it = g_policyKVs.find(k);
        return it == g_policyKVs.end() ? def : it->second;
    };
    auto allow = [](const std::string& v){ return v == "allow" || v == "true" || v == "1"; };
    std::string sandbox = sv("sandbox", "");
    if (sandbox == "strict") env->setSandbox(claw::Environment::SandboxMode::Strict);
    else if (sandbox == "network") env->setSandbox(claw::Environment::SandboxMode::Network);
    else if (sandbox == "full") env->setSandbox(claw::Environment::SandboxMode::Full);
    std::string fr = sv("file.read", "");
    std::string fw = sv("file.write", "");
    std::string fd = sv("file.delete", "");
    std::string in = sv("input", "");
    std::string out = sv("output", "");
    std::string net = sv("network", "");
    std::string logp = sv("log.path", "");
    std::string logk = sv("log.hmac", "");
    std::string logm = sv("log.meta.required", "");
    std::string ioenc = sv("io.encrypt.default", "");
    std::string iopass = sv("io.pass", "");
    std::string idsStack = sv("ids.stack.max", "");
    std::string idsAlloc = sv("ids.alloc.rate.max", "");
    std::string antiDbg = sv("anti.debug.enforce", "");
    std::string vmBlock = sv("vm.detect.block", "");
    if (!fr.empty() || !fw.empty() || !fd.empty() || !in.empty() || !out.empty() || !net.empty()) {
        claw::Environment::SandboxMode mode = env->sandbox();
        env->setSandbox(mode);
        if (!fr.empty()) env->setFileReadAllowed(allow(fr));
        if (!fw.empty()) env->setFileWriteAllowed(allow(fw));
        if (!fd.empty()) env->setFileDeleteAllowed(allow(fd));
        if (!in.empty()) env->setInputAllowed(allow(in));
        if (!out.empty()) env->setOutputAllowed(allow(out));
        if (!net.empty()) env->setNetworkAllowed(allow(net));
    }
    if (!logp.empty()) env->setLogPath(logp);
    if (!logk.empty()) env->setLogHmacKey(logk);
    if (!logm.empty()) env->setLogMetaRequired(allow(logm));
    if (!ioenc.empty()) env->setDefaultEncryptedIO(allow(ioenc));
    if (!iopass.empty()) env->setIoEncPass(iopass);
    if (!idsStack.empty()) { try { claw::gRuntimeFlags.idsStackMax = std::stoi(idsStack); claw::gRuntimeFlags.idsEnabled = true; } catch (...) {} }
    if (!idsAlloc.empty()) { try { claw::gRuntimeFlags.idsAllocRateMax = static_cast<uint64_t>(std::stoull(idsAlloc)); claw::gRuntimeFlags.idsEnabled = true; } catch (...) {} }
#ifdef _WIN32
    if (!antiDbg.empty()) env->setAntiDebugEnforced(allow(antiDbg));
    if (env->antiDebugEnforced()) {
        if (IsDebuggerPresent()) { std::cerr << "Debugger detected (policy)\n"; std::exit(90); }
    }
    if (allow(vmBlock)) {
        bool vm = false;
        HKEY hKey = nullptr;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char buf[256]; DWORD sz = sizeof(buf);
            if (RegQueryValueExA(hKey, "SystemBiosVersion", nullptr, nullptr, reinterpret_cast<LPBYTE>(buf), &sz) == ERROR_SUCCESS) {
                std::string s(buf, buf + strnlen(buf, sizeof(buf)));
                if (s.find("VMware") != std::string::npos || s.find("VirtualBox") != std::string::npos || s.find("QEMU") != std::string::npos) vm = true;
            }
            RegCloseKey(hKey);
        }
        int cpuInfo[4] = {0};
        __cpuid(cpuInfo, 0x40000000);
        if (cpuInfo[0] != 0) vm = true;
        if (vm) { std::cerr << "Virtualized environment detected (policy)\n"; std::exit(91); }
    }
#endif
}

// Helper to print tokens for debug mode
void dumpTokens(const std::vector<claw::Token>& tokens) {
    std::cout << "\n=== TOKENS ===\n";
    for (const auto& tok : tokens) {
        std::cout << "[" << tok.line << ":" << tok.column << "] "
                  << claw::tokenName(tok.type) << " '" << tok.lexeme << "'";
        if (tok.type == claw::TokenType::String && !tok.stringValue.empty()) {
            std::cout << " -> \"" << tok.stringValue << "\"";
        }
        std::cout << "\n";
    }
    std::cout << "==============\n\n";
}

// Helper to print statements for debug mode
void dumpStatements(const std::vector<claw::StmtPtr>& statements) {
    std::cout << "\n=== AST ===\n";
    for (size_t i = 0; i < statements.size(); i++) {
        std::cout << i + 1 << ": ";
        if (auto* exprStmt = dynamic_cast<claw::ExprStmt*>(statements[i].get())) {
            std::cout << "ExprStmt: " << claw::printAST(exprStmt->expr.get());
        } else if (auto* printStmt = dynamic_cast<claw::PrintStmt*>(statements[i].get())) {
            std::cout << "PrintStmt: " << claw::printAST(printStmt->expr.get());
        } else if (auto* letStmt = dynamic_cast<claw::LetStmt*>(statements[i].get())) {
            std::cout << "LetStmt: " << letStmt->name;
            if (letStmt->initializer) {
                std::cout << " = " << claw::printAST(letStmt->initializer.get());
            }
        } else if (dynamic_cast<claw::IfStmt*>(statements[i].get())) {
            std::cout << "IfStmt";
        } else if (dynamic_cast<claw::WhileStmt*>(statements[i].get())) {
            std::cout << "WhileStmt";
        } else if (dynamic_cast<claw::ForStmt*>(statements[i].get())) {
            std::cout << "ForStmt";
        } else if (auto* fnStmt = dynamic_cast<claw::FnStmt*>(statements[i].get())) {
            std::cout << "FnStmt: " << fnStmt->name << "(";
            for (size_t j = 0; j < fnStmt->parameters.size(); j++) {
                if (j > 0) std::cout << ", ";
                std::cout << fnStmt->parameters[j];
            }
            std::cout << ")";
        } else if (dynamic_cast<claw::ReturnStmt*>(statements[i].get())) {
            std::cout << "ReturnStmt";
        } else if (dynamic_cast<claw::BreakStmt*>(statements[i].get())) {
            std::cout << "BreakStmt";
        } else if (dynamic_cast<claw::ContinueStmt*>(statements[i].get())) {
            std::cout << "ContinueStmt";
        } else if (dynamic_cast<claw::BlockStmt*>(statements[i].get())) {
            std::cout << "BlockStmt";
        } else {
            std::cout << "Unknown";
        }
        std::cout << "\n";
    }
    std::cout << "===========\n\n";
}

void printRuntimeError(const claw::RuntimeError& e) {
    std::cerr << "❌ " << claw::errorCodeToString(e.code) << ": Runtime Error [Line " << e.token.line 
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

void runFile(const std::string& path, claw::Interpreter& interpreter, bool debugMode) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << "\n";
        exit(74);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    // Tokenize
    claw::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    // Debug: print tokens
    if (debugMode) {
        dumpTokens(tokens);
    }
    
    // Parse
    claw::Parser parser(tokens);
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
    } catch (const claw::RuntimeError& e) {
        printRuntimeError(e);
        exit(70);
    }
}

bool compileFileToChunk(const std::string& path, std::unique_ptr<claw::Chunk>& chunk, bool debugMode) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    claw::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    if (debugMode) {
        dumpTokens(tokens);
    }

    claw::Parser parser(tokens);
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

    claw::Compiler compiler;
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
    claw::Interpreter interpreter;
    loadVoltsecPolicy(std::filesystem::current_path());
    applyPolicyToEnv(interpreter.getGlobals());
    interpreter.getGlobals()->setSandbox(g_cliSandboxMode);
    std::vector<std::string> history;
    std::string buffer;
    
    std::cout << "\n⚡ ClawScript v" << claw::CLAW_VERSION << " REPL\n";
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
            if (line == "why") {
                std::cout << "ClawScript (formerly VoltScript): unified naming, .claw files only.\n";
                std::cout << "Security policy via .voltsec; IDS/IPS and anti-reverse controls integrated.\n";
                continue;
            }
            if (line == "clear") {
                interpreter.reset();
                std::cout << "Environment cleared.\n";
                continue;
            }
            if (line == "help") {
                std::cout << "\n=== ClawScript v" << claw::CLAW_VERSION << " Help ===\n";
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
            claw::Lexer lexer(buffer);
            auto tokens = lexer.tokenize();
            
            // Parse
            claw::Parser parser(tokens);
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
            
        } catch (const claw::RuntimeError& e) {
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
    if (argc >= 2) {
        std::string cmd = argv[1];
        if (cmd == "init") {
            if (argc < 3) {
                std::cerr << "Usage: claw init <project>\n";
                return 64;
            }
            std::filesystem::path dir = argv[2];
            try {
                std::filesystem::create_directories(dir);
                std::ofstream mv((dir / "main.claw").string());
                mv << "print(\"Hello, Claw!\")\n";
                std::ofstream cj((dir / "claw.json").string());
                cj << "{\n  \"name\": \"" << dir.filename().string() << "\",\n  \"version\": \"0.1.0\"\n}\n";
            } catch (...) {
                std::cerr << "Project init failed\n";
                return 74;
            }
            return 0;
        } else if (cmd == "build") {
            if (argc < 3) {
                std::cerr << "Usage: claw build <script>\n";
                return 64;
            }
            std::string in = argv[2];
            std::unique_ptr<claw::Chunk> chunk;
            if (!compileFileToChunk(in, chunk, false)) {
                return 65;
            }
            std::filesystem::path p(in);
            std::filesystem::path out = p;
            out.replace_extension(".vbc");
            try {
                std::ofstream b(out.string(), std::ios::binary);
                const auto& code = chunk->code();
                b.write(reinterpret_cast<const char*>(code.data()), (std::streamsize)code.size());
            } catch (...) {
                std::cerr << "Bytecode write failed\n";
            }
#ifdef CLAW_ENABLE_AOT
            claw::AotCompiler aotCompiler;
            auto module = aotCompiler.compile("claw_aot", *chunk);
            std::filesystem::path obj = p;
            obj.replace_extension(".o");
            std::ofstream o(obj.string(), std::ios::binary);
            o.write(reinterpret_cast<const char*>(module.image.data()), static_cast<std::streamsize>(module.image.size()));
            if (!o) {
                std::cerr << "Failed to write AOT object\n";
            } else {
#ifdef _WIN32
                std::filesystem::path exe = p;
                exe.replace_extension(".exe");
                std::filesystem::path runtimeLib = std::filesystem::path(argv[0]).parent_path() / "claw_runtime.lib";
                std::string cmdline = "lld-link /OUT:" + exe.string() + " " + obj.string() + " " + runtimeLib.string();
                int linkResult = std::system(cmdline.c_str());
                if (linkResult != 0) {
                    std::cerr << "Linker failed with exit code " << linkResult << "\n";
                }
#else
                std::filesystem::path exe = p;
                exe.replace_extension("");
                std::filesystem::path runtimeLib = std::filesystem::path(argv[0]).parent_path() / "libclaw_runtime.a";
                std::string cmdline = "ld -o " + exe.string() + " " + obj.string() + " " + runtimeLib.string() + " -lstdc++ -lm -lc -lpthread";
                int linkResult = std::system(cmdline.c_str());
                if (linkResult != 0) {
                    std::cerr << "Linker failed with exit code " << linkResult << "\n";
                }
#endif
            }
#endif
            return 0;
        } else if (cmd == "run") {
            if (argc < 3) {
                std::cerr << "Usage: claw run <script>\n";
                return 64;
            }
            jitAggressive = true;
            scriptPath = argv[2];
        }
    }
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            debugMode = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "⚡ ClawScript v" << claw::CLAW_VERSION << "\n";
            std::cout << "Usage: claw [options] [script]\n\n";
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
            std::cout << "  --sandbox=MODE      Set sandbox mode: strict|network|full\n";
            std::cout << "\nCommands:\n";
            std::cout << "  init <project>      Create boilerplate main.claw + claw.json\n";
            std::cout << "  build <script>      Emit bytecode (.vbc) and AOT native\n";
            std::cout << "  run <script>        Run with JIT/AoT hybrid\n";
            return 0;
        } else if (arg == "--version") {
            std::cout << "ClawScript " << claw::CLAW_VERSION << "\n";
            return 0;
        } else if (arg == "--jit=aggressive") {
            jitAggressive = true;
        } else if (arg == "--disable-call-ic") {
            disableCallIC = true;
        } else if (arg == "--ic-diagnostics") {
            icDiagnostics = true;
        } else if (arg.rfind("--sandbox=", 0) == 0) {
            std::string mode = arg.substr(std::string("--sandbox=").size());
            if (mode == "strict") {
                g_cliSandboxMode = claw::Environment::SandboxMode::Strict;
            } else if (mode == "network") {
                g_cliSandboxMode = claw::Environment::SandboxMode::Network;
            } else if (mode == "full") {
                g_cliSandboxMode = claw::Environment::SandboxMode::Full;
            } else {
                std::cerr << "Unknown sandbox mode: " << mode << "\n";
                return 64;
            }
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
        claw::gJitConfig.aggressive = true;
        claw::gJitConfig.loopThreshold = 1000;
        claw::gJitConfig.functionThreshold = 1000;
    }
    const char* envDisable = std::getenv("CLAW_DISABLE_CALL_IC");
    if (!envDisable) envDisable = std::getenv("VOLT_DISABLE_CALL_IC");
    const char* envDiag = std::getenv("CLAW_IC_DIAGNOSTICS");
    if (!envDiag) envDiag = std::getenv("VOLT_IC_DIAGNOSTICS");
    const char* envProfile = std::getenv("CLAW_PROFILE");
    if (!envProfile) envProfile = std::getenv("VOLT_PROFILE");
    const char* envProfileHz = std::getenv("CLAW_PROFILE_HZ");
    if (!envProfileHz) envProfileHz = std::getenv("VOLT_PROFILE_HZ");
    const char* envProfileOut = std::getenv("CLAW_PROFILE_OUT");
    if (!envProfileOut) envProfileOut = std::getenv("VOLT_PROFILE_OUT");
    if (envDisable && *envDisable) disableCallIC = true;
    if (envDiag && *envDiag) icDiagnostics = true;
    if (!enableProfile && envProfile && *envProfile) enableProfile = true;
    if (envProfileHz && *envProfileHz) { try { profileHz = std::stoi(envProfileHz); } catch (...) {} }
    if (profileOutput.empty() && envProfileOut && *envProfileOut) profileOutput = envProfileOut;
    claw::gRuntimeFlags.disableCallIC = disableCallIC;
    claw::gRuntimeFlags.icDiagnostics = icDiagnostics;
    
    if (!aotOutputPath.empty()) {
        if (scriptPath.empty()) {
            std::cerr << "--aot-output requires a script file\n";
            return 64;
        }

#ifdef CLAW_ENABLE_AOT
        std::unique_ptr<claw::Chunk> chunk;
        if (!compileFileToChunk(scriptPath, chunk, debugMode)) {
            return 65;
        }

        claw::AotCompiler aotCompiler;
        auto module = aotCompiler.compile("claw_aot", *chunk);

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
        std::filesystem::path runtimeLib = std::filesystem::path(argv[0]).parent_path() / "claw_runtime.lib";
        std::string cmd = "lld-link /OUT:" + exePath.string() + " " + objPath.string() + " " + runtimeLib.string();
#else
        std::filesystem::path runtimeLib = std::filesystem::path(argv[0]).parent_path() / "libclaw_runtime.a";
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

    claw::Interpreter interpreter;
    if (!scriptPath.empty()) {
        loadVoltsecPolicy(std::filesystem::path(scriptPath).parent_path());
    } else {
        loadVoltsecPolicy(std::filesystem::current_path());
    }
    applyPolicyToEnv(interpreter.getGlobals());
    interpreter.getGlobals()->setSandbox(g_cliSandboxMode);
    if (enableProfile) {
        claw::profilerSetCurrentInterpreter(&interpreter);
        claw::profilerStart(profileHz);
    }
    
    if (!scriptPath.empty()) {
        // Run file
        runFile(scriptPath, interpreter, debugMode);
    } else {
        // Interactive REPL
        runPrompt();
    }
    if (enableProfile || claw::profilerEnabled()) {
        claw::profilerStop();
        claw::Profiler::instance().writeHtml(profileOutput);
        claw::Profiler::instance().writeSpeedscope(profileOutput);
    }
    
    return 0;
}
