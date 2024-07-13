#include "evaluate.h"
#include <vector>
#include <stdexcept>
#include <format>

namespace SYP {

Result toResult(const Token &token) {
  switch (token.type) {
    case TokenType::Boolean: return token.boolValue;
    case TokenType::Unsigned: return token.unsignedValue;
    case TokenType::Signed: return token.signedValue;
    case TokenType::Float: return token.floatValue;
    case TokenType::String: return token.getVariableName();
    default: throw std::runtime_error(std::format("invalid result type {}", static_cast<int>(token.type)));
  }
}

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::string toString(const Result& result) {
  return std::visit(overloaded{
    [](int64_t as_int) { return std::to_string(as_int); },
    [](uint64_t as_uint) { return std::to_string(as_uint); },
    [](double as_double) { return std::to_string(as_double); },
    [](bool as_bool) { return std::to_string(as_bool); },
    [](std::string as_str) { return std::move(as_str); }
    }, result);
}

std::string toString(const Token& token) {
  switch (token.type) {
    case TokenType::Boolean: return std::to_string(token.boolValue);
    case TokenType::Unsigned: return std::to_string(token.unsignedValue);
    case TokenType::Signed: return std::to_string(token.signedValue);
    case TokenType::Float: return std::to_string(token.floatValue);
    case TokenType::String: return std::format("variable: {}", token.getVariableName());
    default: throw std::runtime_error("invalid token type");
  }
}

Token noVariables(const std::string&) {
  throw std::runtime_error("no variables support");
}

void noAssign(const std::string&, const Token&) {
  throw std::runtime_error("read-only function");
}

inline void push(TokenStack &stack, Token &&token) {
  stack.first[stack.second++] = std::move(token);
}

Result evaluate(const TokenQueue &tokens, const std::function<Token(const std::string&)> &resolve, const std::function<void(const std::string&, const Token&)> &assign) {
  thread_local static TokenStack stack = TokenStack{ std::make_pair(std::vector<Token>{}, 0) };

  if (tokens.size() > stack.first.size()) {
    stack.first.resize(tokens.size());
  }

  stack.second = 0;

  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto& cur = tokens[i];

    if (cur.type == TokenType::Operator) {
      push(stack, cur.evaluate(stack, resolve, assign));
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
      auto&& funcToken = resolve(cur.getVariableName());
      funcToken.type = TokenType::Function;
      push(stack, std::move(funcToken));
    } else {
      push(stack, Token(cur));
    }
  }

  if (stack.second != 1) {
    throw std::runtime_error("failed to evaluate term");
  }

  if (stack.first[0].type == TokenType::Variable)
  {
    return toResult(resolve(stack.first[0].getVariableName()));
  }

  return toResult(stack.first[0]);
}

}
