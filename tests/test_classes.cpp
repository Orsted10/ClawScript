#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <sstream>

namespace volt {

class ClassSystemTest : public ::testing::Test {
protected:
    std::string runCode(const std::string& source) {
        // Redirect cout to capture output
        std::streambuf* oldCout = std::cout.rdbuf();
        std::stringstream capturedOutput;
        std::cout.rdbuf(capturedOutput.rdbuf());

        try {
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto statements = parser.parseProgram();

            if (parser.hadError()) {
                std::cout.rdbuf(oldCout);
                std::string errs;
                for (const auto& e : parser.getErrors()) errs += e + "\n";
                return "Parser Error: " + errs;
            }

            Interpreter interpreter;
            interpreter.execute(statements);
        } catch (const RuntimeError& e) {
            std::cout.rdbuf(oldCout);
            return std::string("Runtime Error: ") + e.what();
        } catch (const std::exception& e) {
            std::cout.rdbuf(oldCout);
            return std::string("Exception: ") + e.what();
        }

        std::cout.rdbuf(oldCout);
        return capturedOutput.str();
    }
};

TEST_F(ClassSystemTest, BasicClassDeclaration) {
    std::string code = 
        "class Person {\n"
        "  fn init(name) {\n"
        "    this.name = name;\n"
        "  }\n"
        "  fn sayHello() {\n"
        "    print \"Hello, I am \" + this.name;\n"
        "  }\n"
        "}\n"
        "let p = Person(\"Alice\");\n"
        "p.sayHello();\n";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "Hello, I am Alice\n");
}

TEST_F(ClassSystemTest, ClassInheritance) {
    std::string code = 
        "class Animal {\n"
        "  fn init(name) {\n"
        "    this.name = name;\n"
        "  }\n"
        "  fn speak() {\n"
        "    print this.name + \" makes a sound\";\n"
        "  }\n"
        "}\n"
        "class Dog < Animal {\n"
        "  fn speak() {\n"
        "    print this.name + \" barks\";\n"
        "  }\n"
        "}\n"
        "let d = Dog(\"Rex\");\n"
        "d.speak();\n";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "Rex barks\n");
}

TEST_F(ClassSystemTest, SuperMethodCall) {
    std::string code = 
        "class Animal {\n"
        "  fn speak() {\n"
        "    print \"Animal sound\";\n"
        "  }\n"
        "}\n"
        "class Dog < Animal {\n"
        "  fn speak() {\n"
        "    super.speak();\n"
        "    print \"Woof!\";\n"
        "  }\n"
        "}\n"
        "let d = Dog();\n"
        "d.speak();\n";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "Animal sound\nWoof!\n");
}

TEST_F(ClassSystemTest, FieldAccess) {
    std::string code = 
        "class Box {}\n"
        "let b = Box();\n"
        "b.content = \"gold\";\n"
        "print b.content;\n";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "gold\n");
}

TEST_F(ClassSystemTest, Constructor) {
    std::string code = 
        "class Point {\n"
        "  fn init(x, y) {\n"
        "    this.x = x;\n"
        "    this.y = y;\n"
        "  }\n"
        "}\n"
        "let p = Point(10, 20);\n"
        "print p.x;\n"
        "print p.y;\n";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n20\n");
}

} // namespace volt
