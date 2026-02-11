#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "ast.h"
#include "stmt.h"
#include "token.h"
#include "version.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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
    std::cerr << "❌ Runtime Error [Line " << e.token.line 
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
            return 0;
        } else if (arg == "--version") {
            std::cout << "VoltScript " << volt::VOLT_VERSION << "\n";
            return 0;
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
    
    volt::Interpreter interpreter;
    
    if (!scriptPath.empty()) {
        // Run file
        runFile(scriptPath, interpreter, debugMode);
    } else {
        // Interactive REPL
        runPrompt();
    }
    
    return 0;
}
