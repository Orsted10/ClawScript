#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "VoltScript Milestone 3: Parser\n";
        std::cout << "Usage: volt \"expression\"\n";
        std::cout << "Examples:\n";
        std::cout << "  volt \"1 + 2 * 3\"\n";
        std::cout << "  volt \"(10 + 5) / 3\"\n";
        std::cout << "  volt \"x = 5 * 2\"\n";
        std::cout << "  volt \"a == b && c != d\"\n";
        return 0;
    }
    
    std::string input = argv[1];
    
    // Tokenize
    volt::Lexer lexer(input);
    auto tokens = lexer.tokenize();
    
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        if (token.type != volt::TokenType::Eof) {
            std::cout << token.lexeme << " ";
        }
    }
    std::cout << "\n";
    
    // Parse
    volt::Parser parser(tokens);
    auto ast = parser.parse();
    
    if (parser.hadError()) {
        std::cout << "\nParse Errors:\n";
        for (const auto& error : parser.getErrors()) {
            std::cout << "  " << error << "\n";
        }
        return 1;
    }
    
    if (ast) {
        std::cout << "AST: " << volt::printAST(ast.get()) << "\n";
    }
    
    return 0;
}
