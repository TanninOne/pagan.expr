#include "evaluate.h"
#include <vector>
#include <stdexcept>

namespace SYP {

Result toResult(const Token &token) {
  switch (token.type) {
  case TokenType::Boolean:
    return token.boolValue;
  case TokenType::Unsigned:
    return token.unsignedValue;
  case TokenType::Signed:
    return token.signedValue;
  case TokenType::Float:
    return token.floatValue;
  case TokenType::String:
    return token.getVariableName();
  default:
    throw std::runtime_error("invalid result type");
  }
}

Token noVariables(const std::string &) {
  throw std::runtime_error("no variables support");
}

inline void push(TokenStack &stack, Token &&token) {
  stack.first[stack.second++] = std::move(token);
}

Result evaluate(const TokenQueue &tokens,
                const std::function<Token(const std::string &)> &resolve) {
  thread_local static TokenStack stack =
      TokenStack{std::make_pair(std::vector<Token>{}, 0)};

  if (tokens.size() > stack.first.size()) {
    stack.first.resize(tokens.size());
  }

  stack.second = 0;

  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto &cur = tokens[i];

    if (cur.type == TokenType::Operator) {
      push(stack, cur.evaluate(stack));
    } else if (cur.type == TokenType::Variable) {
      push(stack, resolve(cur.getVariableName()));
    } else if (cur.type == TokenType::FunctionName) {
      /*
      auto function = cur.getFunction();
      if (function == nullptr) {
        auto funcToken = resolve(cur.getVariableName());
        function = funcToken.getFunction();
      }
      std::vector<Token> arguments;
      ++i;
      while ((tokens[i].type != TokenType::Operator) ||
             (tokens[i].op != OperatorType::ArgumentList)) {
        arguments.emplace_back(tokens[i++]);
      }

      push(stack, function(arguments));
      */
      push(stack, resolve(cur.getVariableName()));
    } else {
      push(stack, Token(cur));
    }
  }

  if (stack.second != 1) {
    throw std::runtime_error("failed to evaluate term");
  }

  return toResult(stack.first[0]);
}

} // namespace SYP
