#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "formatter.h"

using namespace volt;

static std::vector<std::filesystem::path> collectVoltFiles(const std::filesystem::path& root) {
    std::vector<std::filesystem::path> files;
    for (auto& p : std::filesystem::recursive_directory_iterator(root)) {
        if (!p.is_regular_file()) continue;
        if (p.path().extension() == ".volt") files.push_back(p.path());
    }
    return files;
}

int main(int argc, char** argv) {
    bool write = false;
    bool check = false;
    std::filesystem::path start = std::filesystem::current_path();
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--write") write = true;
        else if (arg == "--check") check = true;
        else if (arg.rfind("--root=", 0) == 0) {
            start = arg.substr(7);
        }
    }
    auto files = collectVoltFiles(start);
    bool anyChanged = false;
    for (const auto& file : files) {
        std::ifstream in(file.string(), std::ios::binary);
        if (!in) continue;
        std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        std::string formatted = fmt::formatTokens(tokens, 2);
        if (formatted != source) {
            anyChanged = true;
            if (write) {
                std::ofstream out(file.string(), std::ios::binary | std::ios::trunc);
                out << formatted;
            } else {
                std::cout << "Would reformat: " << file.string() << "\n";
            }
        }
    }
    if (check) {
        return anyChanged ? 2 : 0;
    }
    return 0;
}
