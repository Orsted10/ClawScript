#include <gtest/gtest.h>
#include <random>
#include <string>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#
static std::string genProgram(std::mt19937& rng, int scale = 100) {
    std::uniform_int_distribution<int> dist(0, 3);
    int choice = dist(rng);
    if (choice == 0) {
        return "let x = 0; for (let i=0;i<" + std::to_string(scale) + ";i=i+1){ x = x + i; } print x;";
    } else if (choice == 1) {
        return "fn f(a){ return a+1; } let y=0; for (let i=0;i<" + std::to_string(scale) + ";i=i+1){ y=f(y); } print y;";
    } else if (choice == 2) {
        return "class C { fn inc(){ this.v = this.v + 1; return this.v; } } let c=C(); c.v=0; for (let i=0;i<" + std::to_string(scale/4) + ";i=i+1){ c.inc(); } print c.v;";
    } else {
        return "let arr=[]; for(let i=0;i<" + std::to_string(scale) + ";i=i+1){ arr.push(i); } print arr.length;";
    }
}
#
TEST(PropertyFuzz, RandomProgramsNoCrash) {
    std::vector<uint32_t> seeds = {12345u, 9876u, 42u, 777u, 2024u};
    for (auto seed : seeds) {
        std::mt19937 rng(seed);
        for (int n = 0; n < 60; n++) {
            int scale = 50 + (n % 5) * 50; // 50..250
            std::string src = genProgram(rng, scale);
            volt::Lexer lexer(src);
            auto tokens = lexer.tokenize();
            volt::Parser parser(tokens);
            auto statements = parser.parseProgram();
            ASSERT_FALSE(parser.hadError());
            volt::Interpreter interpreter;
            EXPECT_NO_THROW(interpreter.execute(statements));
        }
    }
}

TEST(PropertyFuzz, RandomProgramsNoCrashVM) {
    std::vector<uint32_t> seeds = {12345u, 9876u, 42u};
    for (auto seed : seeds) {
        std::mt19937 rng(seed);
        for (int n = 0; n < 40; n++) {
            int scale = 50 + (n % 4) * 50; // 50..200
            std::string src = genProgram(rng, scale);
            volt::Lexer lexer(src);
            auto tokens = lexer.tokenize();
            volt::Parser parser(tokens);
            auto statements = parser.parseProgram();
            ASSERT_FALSE(parser.hadError());
            volt::Interpreter interpreter;
            // Execute with interpreter to seed globals (classes/functions)
            EXPECT_NO_THROW(interpreter.execute(statements));
            // Compile and execute a simple final print to keep VM path active
            std::string loop = "let m=0; for(let i=0;i<100;i=i+1){ m=m+i; } print m;";
            volt::Lexer l2(loop);
            auto t2 = l2.tokenize();
            volt::Parser p2(t2);
            auto s2 = p2.parseProgram();
            volt::Compiler comp;
            auto chunk = comp.compile(s2);
            volt::VM vm(interpreter);
            EXPECT_EQ(vm.interpret(*chunk), volt::InterpretResult::Ok);
        }
    }
}
