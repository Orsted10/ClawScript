#pragma once
#include <string>
#include <vector>
#include "lexer/token.h"

namespace claw::fmt {

std::string formatTokens(const std::vector<claw::Token>& tokens, int indentSize = 2);

} // namespace claw::fmt
