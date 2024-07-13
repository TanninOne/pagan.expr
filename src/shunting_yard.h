#pragma once

#include <vector>
#include <string_view>
#include "token.h"

namespace SYP {

[[nodiscard]] std::vector<Token> tokenize(std::string_view input);

// const Token& numericalFromString(std::string_view& view, double& value, bool isNegative);

}
