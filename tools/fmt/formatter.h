#pragma once
#include <string>
#include <vector>
#include "lexer/token.h"

namespace volt::fmt {

std::string formatTokens(const std::vector<volt::Token>& tokens, int indentSize = 2);

} // namespace volt::fmt
