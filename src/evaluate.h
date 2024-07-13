#pragma once

#include <cstdint>
#include <variant>
#include <functional>

#include "token.h"

namespace SYP {

using Result = std::variant<int64_t, uint64_t, double, bool, std::string>;

Token noVariables(const std::string&);
void noAssign(const std::string&, const Token&);

std::string toString(const Result& result);
std::string toString(const Token& token);

Result evaluate(const TokenQueue &tokens, const std::function<Token(const std::string&)> &resolve = noVariables, const std::function<void(const std::string&, const Token&)> &assign = noAssign);

}
