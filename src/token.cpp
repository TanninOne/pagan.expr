#include "token.h"

#include "evaluate.h"

#include <cstdint>
#include <format>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <array>

using namespace std::string_literals;

namespace SYP {

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
  case OperatorType::Assign:
    return 30;
  case OperatorType::BracketOpen:
  case OperatorType::BracketClose:
  case OperatorType::ArgumentList:
    // brackets are handled separately so this priority should be so high
    // it never gets removed from operator stack prematurely
    return 20;
  default:
    throw std::out_of_range(
        std::format("Invalid operator type {}", static_cast<unsigned>(op)));
  }

}

template<typename T>
const std::type_info& variant_type(const T& variant) {
  return std::visit([](auto&& value) -> decltype(auto) { return typeid(value); }, variant);
}

/*
template <typename T> T pop(TokenStack& args) {
  const auto &tmp = args.first[--args.second].value;

  try {
    return std::get<T>(tmp);
  }
  catch (const std::bad_variant_access& e)
  {
    throw std::runtime_error(std::format("incompatible types: {} vs {}", typeid(T).name(), variant_type(tmp).name()));
  }
}
*/

template <typename T> T tokenTo(const Token& tok, const std::function<Token(const std::string&)>& resolve)
{
  if constexpr (std::is_same_v<uint64_t, T>)
    return tok.unsignedValue;
  else if constexpr (std::is_same_v<int64_t, T>)
    return tok.signedValue;
  else if constexpr (std::is_same_v<double, T>)
    return tok.floatValue;
  else if constexpr (std::is_same_v<bool, T>)
    return tok.boolValue;
  else if constexpr (std::is_same_v<std::string, T>)
    return (tok.type == TokenType::String) ? tok.getVariableName() : resolve(tok.getVariableName()).getVariableName();
  else if constexpr (std::is_same_v<OperatorType, T>)
    return tok.op;
  else
    throw std::runtime_error(std::format("unsupported type {}", typeid(T).name()));
}

Token nop(const std::string&)
{
  throw std::runtime_error("unexpected unresolved variable");
}

template <typename T> T pop(TokenStack& args)
{
  const auto &tok = args.first[--args.second];
  return tokenTo<T>(tok, nop);
}

#define resolveToken(token) \
  resolveTokenImpl(token, resolve)

//    token.type == TokenType::Variable ? resolve(token.getVariableName()) : token

Token resolveTokenImpl(const Token& token, const std::function<Token(const std::string&)>& resolve)
{
  if (token.type == TokenType::Variable)
  {
    return resolve(token.getVariableName());
  }
  else {
    return token;
  }
}

#define BINARY_OP(op)                                                          \
  auto rhs = resolveToken(args.first[--args.second]);                          \
  auto lhs = resolveToken(args.first[--args.second]);                          \
  switch (lhs.type) {                                                          \
  case TokenType::Unsigned:                                                    \
    return tokenTo<uint64_t>(lhs, resolve) op tokenTo<uint64_t>(rhs, resolve); \
  case TokenType::Signed:                                                      \
    return tokenTo<int64_t>(lhs, resolve) op tokenTo<int64_t>(rhs, resolve);   \
  case TokenType::Float:                                                       \
    return tokenTo<double>(lhs, resolve) op tokenTo<double>(rhs, resolve);     \
  default:                                                                     \
    throw std::runtime_error("invalid token type for arithmetic op");          \
  }

#define BINARY_STR_OP(op)                                                      \
  auto rhs = resolveToken(args.first[--args.second]);                          \
  auto lhs = resolveToken(args.first[--args.second]);                          \
  std::cout << "add " << toString(lhs) << " and " << toString(rhs)             \
            << std::endl;                                                      \
  switch (lhs.type) {                                                          \
  case TokenType::Unsigned:                                                    \
    return tokenTo<uint64_t>(lhs, resolve) op tokenTo<uint64_t>(rhs, resolve); \
  case TokenType::Signed:                                                      \
    return tokenTo<int64_t>(lhs, resolve) op tokenTo<int64_t>(rhs, resolve);   \
  case TokenType::Float:                                                       \
    return tokenTo<double>(lhs, resolve) op tokenTo<double>(rhs, resolve);     \
  case TokenType::String:                                                      \
    return Token(tokenTo<std::string>(lhs, resolve)                            \
                     op tokenTo<std::string>(rhs, resolve),                    \
                 TokenType::String);                                           \
  default:                                                                     \
    throw std::runtime_error("invalid token type for binary op");              \
  }

#define BINARY_UNSIGNED_OP(op)                                                 \
  auto rhs = resolveToken(args.first[--args.second]);                          \
  auto lhs = resolveToken(args.first[--args.second]);                          \
  switch (lhs.type) {                                                          \
  case TokenType::Unsigned:                                                    \
    return tokenTo<uint64_t>(lhs, resolve) op tokenTo<uint64_t>(rhs, resolve); \
  case TokenType::Signed:                                                      \
    return static_cast<uint64_t>(tokenTo<int64_t>(lhs, resolve))               \
        op static_cast<uint64_t>(tokenTo<int64_t>(rhs, resolve));              \
  default:                                                                     \
    throw std::runtime_error("invalid token type for arithmetic");             \
  }

#define BINARY_LOGICAL_OP(op)                                                  \
  auto rhs = resolveToken(args.first[--args.second]);                          \
  auto lhs = resolveToken(args.first[--args.second]);                          \
  switch (lhs.type) {                                                          \
  case TokenType::Boolean:                                                     \
    return tokenTo<bool>(lhs, resolve) op tokenTo<bool>(rhs, resolve);         \
  case TokenType::Unsigned:                                                    \
    return (tokenTo<uint64_t>(lhs, resolve) != 0)                              \
        op(tokenTo<uint64_t>(rhs, resolve) != 0);                              \
  case TokenType::Signed:                                                      \
    return (tokenTo<int64_t>(lhs, resolve) != 0)                               \
        op(tokenTo<int64_t>(rhs, resolve) != 0);                               \
  default:                                                                     \
    throw std::runtime_error("invalid token type for logical");                \
  }

#define UNARY_OP(op)                                                           \
  auto operand = resolveToken(args.first[--args.second]);                      \
  switch (operand.type) {                                                      \
  case TokenType::Boolean:                                                     \
    return op tokenTo<bool>(operand, resolve);                                 \
  default:                                                                     \
    throw std::runtime_error("invalid token type for logical");                \
  }

static const std::array<std::function<Token(TokenStack &, const std::function<Token(const std::string &)> &resolve)>,
                        static_cast<unsigned>(OperatorType::OperatorCount)>
    s_Operations{
        /*Invalid */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { throw std::runtime_error("trying to evaluate invalid operator"); },
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { throw std::runtime_error("assignment not implemented yet"); },
        /*Add */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_STR_OP(+) },
        /*Subtract */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(-) },
        /*Multiply */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(*) },
        /*Divide */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(/) },
        /*Modulo */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_UNSIGNED_OP(%) },

        /*ShiftLeft */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_UNSIGNED_OP(<<) },
        /*ShiftRight */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_UNSIGNED_OP(>>) },
        /*Xor */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_UNSIGNED_OP(^) },
        /*BitwiseAnd */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_UNSIGNED_OP(&) },
        /*BitwiseOr */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_UNSIGNED_OP(|) },

        /*LessThan */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(<) },
        /*LessOrEqual */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(<=) },
        /*GreaterThan */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(>) },
        /*GreaterOrEqual */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(>=) },

       /*Equal */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(==) },
        /*NotEqual */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_OP(!=) },

        /*LogicalAnd */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_LOGICAL_OP(&&) },
        /*LogicalOr */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { BINARY_LOGICAL_OP(||) },
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token { UNARY_OP(!) },

        /*TernaryQ */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token {
          // x ? y : z
          // x ? y is evaluated first, we push y on the stack if the condition is true, an invalid operator otherwise
          const auto &tok = resolveToken(args.first[--args.second]);
          auto cond = pop<bool>(args);
          if (cond) {
            return tok;
          } else {
            return Token(OperatorType::Incomplete);
          }
        },
        /*TernaryE */
        [](TokenStack &args, const std::function<Token(const std::string&)> resolve) -> Token {
          // x ? y : z
          // if x was true, we now see y : z and then y "wins".
          // if x was false, we now see <invalid> : z and then z "wins"
          const auto &rhs = resolveToken(args.first[--args.second]);
          const auto &lhs = resolveToken(args.first[--args.second]);
          if (lhs.type == TokenType::Operator) {
            return rhs;
          } else {
            return lhs;
          }
        }};

KnownVariables Token::s_KnownVariables{};
KnownFunctions Token::s_KnownFunctions{};
uint64_t Token::s_NextVariable{1};

Token Token::evaluate(
    TokenStack &args, const std::function<Token(const std::string &)> &resolve,
    const std::function<void(const std::string &, const Token &)> &assign)
    const {
  // } else if (cur.type == TokenType::Variable) {
  //   push(stack, resolve(cur.getVariableName()));
  if (type != TokenType::Operator) {
    throw std::runtime_error("expected operator token");
  }
  if (op == OperatorType::ArgumentList) {
    std::vector<Token> funcArgs;
    --args.second;
    while (args.first[args.second].type != TokenType::Function) {
      funcArgs.emplace_back(resolve(args.first[args.second--].getVariableName()));
   }
    std::reverse(funcArgs.begin(), funcArgs.end());
    return args.first[args.second].getFunction()(funcArgs);
  } else if (op == OperatorType::Assign) {
    auto rhs = args.first[--args.second];
    if (rhs.type == TokenType::Variable)
    {
      rhs = resolve(rhs.getVariableName());
    }
    const auto &lhs = args.first[--args.second];
    assign(lhs.getVariableName(), rhs);
    return Token(true);
  }
  return s_Operations[static_cast<unsigned>(op)](args, resolve);
}
}
