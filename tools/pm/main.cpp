#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

static const char* kPkgFile = "clawpkg.json";

static void printUsage() {
    std::cout << "claw-pm - ClawScript package manager (stub)\n";
    std::cout << "Usage:\n";
    std::cout << "  claw-pm init                 Initialize clawpkg.json\n";
    std::cout << "  claw-pm add <name> <url>     Add package (stubbed)\n";
    std::cout << "  claw-pm remove <name>        Remove package\n";
    std::cout << "  claw-pm list                 List packages\n";
}

static void ensureInit() {
    if (!std::filesystem::exists(kPkgFile)) {
        std::ofstream o(kPkgFile, std::ios::binary);
        o << "{\n  \"name\": \"app\",\n  \"version\": \"0.1.0\",\n  \"dependencies\": {}\n}\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) { printUsage(); return 64; }
    std::string cmd = argv[1];
    if (cmd == "init") {
        ensureInit();
        std::cout << "Initialized " << kPkgFile << "\n";
        return 0;
    } else if (cmd == "add") {
        if (argc < 4) { printUsage(); return 64; }
        ensureInit();
        std::string name = argv[2];
        std::string url = argv[3];
        // Minimal JSON append (non-robust), suitable for stub usage
        std::ifstream in(kPkgFile, std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (contents.find("\"dependencies\"") == std::string::npos) contents += "\n";
        size_t pos = contents.find("\"dependencies\"");
        if (pos == std::string::npos) {
            contents += "\"dependencies\": {}\n";
            pos = contents.find("\"dependencies\"");
        }
        // Naive injection: append a comment line for human tracking
        contents += "\n// " + name + " -> " + url + "\n";
        std::ofstream out(kPkgFile, std::ios::binary | std::ios::trunc);
        out << contents;
        std::cout << "Added (stub): " << name << " from " << url << "\n";
        return 0;
    } else if (cmd == "remove") {
        if (argc < 3) { printUsage(); return 64; }
        ensureInit();
        std::cout << "Removed (stub): " << argv[2] << "\n";
        return 0;
    } else if (cmd == "list") {
        ensureInit();
        std::ifstream in(kPkgFile, std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::cout << contents << "\n";
        return 0;
    } else {
        printUsage();
        return 64;
    }
}
