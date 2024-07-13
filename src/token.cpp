#include "token.h"
#include <cstdint>
#include <format>
#include <stdexcept>
#include <functional>
#include <array>

namespace SYP {

using namespace std::string_literals;

int getOperatorOrder(const OperatorType &op) {
  switch (op) {
  case OperatorType::LogicalNot:
    return 2;
  case OperatorType::Multiply:
  case OperatorType::Divide:
  case OperatorType::Modulo:
    return 3;
  case OperatorType::Add:
  case OperatorType::Subtract:
    return 4;
  case OperatorType::ShiftLeft:
  case OperatorType::ShiftRight:
    return 5;
  case OperatorType::LessThan:
  case OperatorType::LessOrEqual:
  case OperatorType::GreaterThan:
  case OperatorType::GreaterOrEqual:
    return 6;
  case OperatorType::Equal:
  case OperatorType::NotEqual:
    return 7;
  case OperatorType::BitwiseAnd:
    return 8;
  case OperatorType::Xor:
    return 9;
  case OperatorType::BitwiseOr:
    return 10;
  case OperatorType::LogicalAnd:
    return 11;
  case OperatorType::LogicalOr:
    return 12;
  case OperatorType::TernaryQ:
    return 13;
  case OperatorType::TernaryE:
    return 14;
  case OperatorType::BracketOpen:
  case OperatorType::BracketClose:
  case OperatorType::ArgumentList:
    // brackets are handled separately so this priority should be so high
    // it never gets removed from operator stack prematurely
    return 999;
  default:
    throw std::out_of_range(
        std::format("Invalid operator type {}", static_cast<unsigned>(op)));
  }
}

template <typename T> const std::type_info &variant_type(const T &variant) {
  return std::visit(
      [](auto &&value) -> decltype(auto) { return typeid(value); }, variant);
}

/*
template <typename T> T pop(TokenStack& args) {
  const auto &tmp = args.first[--args.second].value;

  try {
    return std::get<T>(tmp);
  }
  catch (const std::bad_variant_access& e)
  {
    throw std::runtime_error(std::format("incompatible types: {} vs {}",
typeid(T).name(), variant_type(tmp).name()));
  }
}
*/

template <typename T> T tokenTo(const Token &tok) {
  if constexpr (std::is_same_v<uint64_t, T>)
    return tok.unsignedValue;
  else if constexpr (std::is_same_v<int64_t, T>)
    return tok.signedValue;
  else if constexpr (std::is_same_v<double, T>)
    return tok.floatValue;
  else if constexpr (std::is_same_v<bool, T>)
    return tok.boolValue;
  else if constexpr (std::is_same_v<std::string, T>)
    return tok.getVariableName();
  else if constexpr (std::is_same_v<OperatorType, T>)
    return tok.op;
  else
    throw std::runtime_error(
        std::format("unsupported type {}", typeid(T).name()));
}

template <typename T> T pop(TokenStack &args) {
  const auto &tok = args.first[--args.second];
  return tokenTo<T>(tok);
}

#define BINARY_OP(op)                                                          \
  auto rhs = args.first[--args.second];                                        \
  auto lhs = args.first[--args.second];                                        \
  switch (lhs.type) {                                                          \
  case TokenType::Unsigned:                                                    \
    return tokenTo<uint64_t>(lhs) op tokenTo<uint64_t>(rhs);                   \
  case TokenType::Signed:                                                      \
    return tokenTo<int64_t>(lhs) op tokenTo<int64_t>(rhs);                     \
  case TokenType::Float:                                                       \
    return tokenTo<double>(lhs) op tokenTo<double>(rhs);                       \
  default:                                                                     \
    throw std::runtime_error("invalid token type for arithmetic op");          \
  }

#define BINARY_STR_OP(op)                                                      \
  auto rhs = args.first[--args.second];                                        \
  auto lhs = args.first[--args.second];                                        \
  switch (lhs.type) {                                                          \
  case TokenType::Unsigned:                                                    \
    return tokenTo<uint64_t>(lhs) op tokenTo<uint64_t>(rhs);                   \
  case TokenType::Signed:                                                      \
    return tokenTo<int64_t>(lhs) op tokenTo<int64_t>(rhs);                     \
  case TokenType::Float:                                                       \
    return tokenTo<double>(lhs) op tokenTo<double>(rhs);                       \
  case TokenType::String:                                                      \
    return Token(tokenTo<std::string>(lhs) op tokenTo<std::string>(rhs),       \
                 TokenType::String);                                           \
  default:                                                                     \
    throw std::runtime_error("invalid token type for binary op");              \
  }

#define BINARY_UNSIGNED_OP(op)                                                 \
  auto rhs = args.first[--args.second];                                        \
  auto lhs = args.first[--args.second];                                        \
  switch (lhs.type) {                                                          \
  case TokenType::Unsigned:                                                    \
    return tokenTo<uint64_t>(lhs) op tokenTo<uint64_t>(rhs);                   \
  case TokenType::Signed:                                                      \
    return static_cast<uint64_t>(tokenTo<int64_t>(lhs))                        \
        op static_cast<uint64_t>(tokenTo<int64_t>(rhs));                       \
  default:                                                                     \
    throw std::runtime_error("invalid token type for arithmetic");             \
  }

#define BINARY_LOGICAL_OP(op)                                                  \
  auto rhs = args.first[--args.second];                                        \
  auto lhs = args.first[--args.second];                                        \
  switch (lhs.type) {                                                          \
  case TokenType::Boolean:                                                     \
    return tokenTo<bool>(lhs) op tokenTo<bool>(rhs);                           \
  case TokenType::Unsigned:                                                    \
    return (tokenTo<uint64_t>(lhs) != 0) op(tokenTo<uint64_t>(rhs) != 0);      \
  case TokenType::Signed:                                                      \
    return (tokenTo<int64_t>(lhs) != 0) op(tokenTo<int64_t>(rhs) != 0);        \
  default:                                                                     \
    throw std::runtime_error("invalid token type for logical");                \
  }

#define UNARY_OP(op)                                                           \
  auto operand = args.first[--args.second];                                    \
  switch (operand.type) {                                                      \
  case TokenType::Boolean:                                                     \
    return op tokenTo<bool>(operand);                                          \
  default:                                                                     \
    throw std::runtime_error("invalid token type for logical");                \
  }

static const std::array<std::function<Token(TokenStack &)>,
                        static_cast<unsigned>(OperatorType::OperatorCount)>
    s_Operations{
        /*Invalid */
        [](TokenStack &args) -> Token {
          throw std::runtime_error("trying to evaluate invalid operator");
        },
        /*Add */
        [](TokenStack &args) -> Token { BINARY_STR_OP(+) },
        /*Subtract */
        [](TokenStack &args) -> Token { BINARY_OP(-) },
        /*Multiply */
        [](TokenStack &args) -> Token { BINARY_OP(*) },
        /*Divide */
        [](TokenStack &args) -> Token { BINARY_OP(/) },
        /*Modulo */
        [](TokenStack &args) -> Token { BINARY_UNSIGNED_OP(%) },

        /*ShiftLeft */
        [](TokenStack &args) -> Token { BINARY_UNSIGNED_OP(<<) },
        /*ShiftRight */
        [](TokenStack &args) -> Token { BINARY_UNSIGNED_OP(>>) },
        /*Xor */
        [](TokenStack &args) -> Token { BINARY_UNSIGNED_OP(^) },
        /*BitwiseAnd */
        [](TokenStack &args) -> Token { BINARY_UNSIGNED_OP(&) },
        /*BitwiseOr */
        [](TokenStack &args) -> Token { BINARY_UNSIGNED_OP(|) },

        /*LessThan */
        [](TokenStack &args) -> Token { BINARY_OP(<) },
        /*LessOrEqual */
        [](TokenStack &args) -> Token { BINARY_OP(<=) },
        /*GreaterThan */
        [](TokenStack &args) -> Token { BINARY_OP(>) },
        /*GreaterOrEqual */
        [](TokenStack &args) -> Token { BINARY_OP(>=) },

        /*Equal */
        [](TokenStack &args) -> Token { BINARY_OP(==) },
        /*NotEqual */
        [](TokenStack &args) -> Token { BINARY_OP(!=) },

        /*LogicalAnd */
        [](TokenStack &args) -> Token { BINARY_LOGICAL_OP(&&) },
        /*LogicalOr */
        [](TokenStack &args) -> Token { BINARY_LOGICAL_OP(||) },
        [](TokenStack &args) -> Token { UNARY_OP(!) },

        /*TernaryQ */
        [](TokenStack &args) -> Token {
          // x ? y : z
          // x ? y is evaluated first, we push y on the stack if the condition
          // is true, an invalid operator otherwise
          const auto &tok = args.first[--args.second];
          auto cond = pop<bool>(args);
          if (cond) {
            return tok;
          } else {
            return Token(OperatorType::Incomplete);
          }
        },
        /*TernaryE */
        [](TokenStack &args) -> Token {
          // x ? y : z
          // if x was true, we now see y : z and then y "wins".
          // if x was false, we now see <invalid> : z and then z "wins"
          const auto &rhs = args.first[--args.second];
          const auto &lhs = args.first[--args.second];
          if (lhs.type == TokenType::Operator) {
            return rhs;
          } else {
            return lhs;
          }
        }};

KnownVariables Token::s_KnownVariables{};
KnownFunctions Token::s_KnownFunctions{};
uint64_t Token::s_NextVariable{1};

Token Token::evaluate(TokenStack &args) const {
  if (type != TokenType::Operator) {
    throw std::runtime_error("expected operator token");
  }
  if (op == OperatorType::ArgumentList) {
    std::vector<Token> funcArgs;
    --args.second;
    while (args.first[args.second].type != TokenType::Function) {
      funcArgs.emplace_back(args.first[args.second--]);
    }
    std::reverse(funcArgs.begin(), funcArgs.end());
    return args.first[args.second].getFunction()(funcArgs);
  }
  return s_Operations[static_cast<unsigned>(op)](args);
}

} // namespace SYP
