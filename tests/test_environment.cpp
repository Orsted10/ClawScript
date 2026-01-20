#include "environment.h"
#include <cassert>
#include <iostream>
#include <memory>

using namespace volt;

int main() {
    // Global environment
    auto global = std::make_shared<Environment>();
    global->define("x", 10);

    assert(global->get("x") == 10);

    // Re-assign
    global->assign("x", 20);
    assert(global->get("x") == 20);

    // Nested scope (block / function)
    auto local = std::make_shared<Environment>(global);
    local->define("x", 100);

    // Shadowing works
    assert(local->get("x") == 100);
    assert(global->get("x") == 20);

    // Assign should update nearest scope
    local->assign("x", 200);
    assert(local->get("x") == 200);
    assert(global->get("x") == 20);

    // Access parent variable
    global->define("y", 5);
    assert(local->get("y") == 5);

    // Undefined variable error
    try {
        local->get("z");
        assert(false); // should not reach here
    } catch (const std::runtime_error&) {
        // expected
    }

    std::cout << "Environment tests passed!" << std::endl;
    return 0;
}
