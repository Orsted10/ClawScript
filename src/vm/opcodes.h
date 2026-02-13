#pragma once
#include <cstdint>

namespace volt {

enum class OpCode : uint8_t {
    Constant,    // Load constant from chunk
    Nil,         // Load nil
    True,        // Load true
    False,       // Load false
    Pop,         // Pop value from stack
    
    GetGlobal,   // Get global variable
    DefineGlobal,// Define global variable
    SetGlobal,   // Set global variable
    GetLocal,    // Get local variable
    SetLocal,    // Set local variable
    GetUpvalue,  // Get upvalue (for closures)
    SetUpvalue,  // Set upvalue
    
    Equal,       // ==
    Greater,     // >
    Less,        // <
    Add,         // +
    Subtract,    // -
    Multiply,    // *
    Divide,      // /
    Not,         // !
    Negate,      // - (unary)
    
    Print,       // print
    Jump,        // Jump forward
    JumpIfFalse, // Jump forward if false
    Loop,        // Jump backward
    
    Call,        // Call function
    Closure,     // Create closure
    Return,      // Return from function
    
    Class,       // Define class
    Inherit,     // Set up inheritance
    Method,      // Define method
    Invoke,      // Call method directly
    SuperInvoke, // Call super method
    GetProperty, // Get instance property
    SetProperty, // Set instance property
};

} // namespace volt
