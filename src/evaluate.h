#pragma once

#include <cstdint>
#include <variant>
#include <functional>

#include "token.h"

namespace SYP {

using Result = std::variant<int64_t, uint64_t, double, bool, std::string>;

Token noVariables(const std::string &);

Result evaluate(
    const TokenQueue &tokens,
    const std::function<Token(const std::string &)> &resolve = noVariables);

} // namespace SYP
