#ifndef VOLT_ERRORS_H
#define VOLT_ERRORS_H

#include <string>
#include <stdexcept>

namespace volt {

enum class ErrorCode {
    // 1xxx: Syntax/Lexical Errors (Reserved for Parser/Lexer)
    SYNTAX_ERROR = 1001,

    // 2xxx: Type and Value Errors
    TYPE_MISMATCH = 2001,
    NOT_CALLABLE = 2002,
    NOT_INDEXABLE = 2003,
    INVALID_ASSIGNMENT = 2004,

    // 3xxx: Environment/Variable Errors
    UNDEFINED_VARIABLE = 3001,
    REDEFINITION = 3002,

    // 4xxx: Runtime/Arithmetic/Resource Errors
    DIVISION_BY_ZERO = 4001,
    INDEX_OUT_OF_BOUNDS = 4002,
    STACK_OVERFLOW = 4003,
    FILE_IO_ERROR = 4004,
    JSON_ERROR = 4005,
    MATH_ERROR = 4006,
    ARGUMENT_COUNT_MISMATCH = 4007,
    
    UNKNOWN_ERROR = 9999
};

inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::TYPE_MISMATCH: return "E2001";
        case ErrorCode::NOT_CALLABLE: return "E2002";
        case ErrorCode::NOT_INDEXABLE: return "E2003";
        case ErrorCode::INVALID_ASSIGNMENT: return "E2004";
        case ErrorCode::UNDEFINED_VARIABLE: return "E3001";
        case ErrorCode::REDEFINITION: return "E3002";
        case ErrorCode::DIVISION_BY_ZERO: return "E4001";
        case ErrorCode::INDEX_OUT_OF_BOUNDS: return "E4002";
        case ErrorCode::STACK_OVERFLOW: return "E4003";
        case ErrorCode::FILE_IO_ERROR: return "E4004";
        case ErrorCode::JSON_ERROR: return "E4005";
        case ErrorCode::MATH_ERROR: return "E4006";
        case ErrorCode::ARGUMENT_COUNT_MISMATCH: return "E4007";
        case ErrorCode::SYNTAX_ERROR: return "E1001";
        case ErrorCode::UNKNOWN_ERROR:
        default: return "E9999";
    }
}

// Base class for all VoltScript errors
class VoltError : public std::runtime_error {
public:
    ErrorCode code;
    VoltError(ErrorCode code, const std::string& message)
        : std::runtime_error(message), code(code) {}
};

} // namespace volt

#endif // VOLT_ERRORS_H
