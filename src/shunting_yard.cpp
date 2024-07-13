#include "shunting_yard.h"
#include <charconv>
#include <format>
#include <stdexcept>
#include <unordered_map>

namespace SYP {

template <typename T, bool isHex>
auto strToNum(const std::string_view& view, T& value)
{
  if constexpr (isHex) {
    return std::from_chars(view.data(), view.data() + view.size(), value, 16);
  }
  else {
    return std::from_chars(view.data(), view.data() + view.size(), value);
  }
}

template <typename T, bool isHex>
[[nodiscard]] Token numericalFromString(std::string_view& view, bool isNegative)
{
  T value;
  auto [ptr, ec] = strToNum<T, isHex>(view, value);
  if (ec != std::errc{}) {
    throw std::runtime_error(
      std::format("failed to parse number: {}", static_cast<int>(ec)));
  }
  if (isNegative) {
    value = -value;
  }
  return Token(value);
}

inline bool isNumDigit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}

inline bool isHexDigit(char ch)
{
  return ((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f'));
}

[[nodiscard]] Token readNumberToken(std::string_view::const_iterator& pos,
  std::string_view::const_iterator end) {
  bool isFloat = false;
  bool isNegative = false;
  bool isHex = false;

  if (*pos == '-') {
    isNegative = true;
    ++pos;
  }
  if ((*pos == '0') && ((pos + 1) != end) && (*(pos + 1) == 'x')) {
    isHex = true;
    pos += 2;
  }

  std::string_view::const_iterator beg = pos++;

  while ((pos != end) && (isNumDigit(*pos) || (*pos == '.') || (isHex && isHexDigit(*pos)))) {
    if (*pos == '.') {
      if (isFloat) {
        throw std::runtime_error("invalid number format");
      }
      isFloat = true;
    }
    ++pos;
  }

  if ((isFloat && isHex) || (isNegative && isHex)) {
    throw std::runtime_error("invalid number format");
  }

  std::string_view view(beg, pos);
  if (isFloat) {
    return numericalFromString<double, false>(view, isNegative);
  }
  else if (isHex) {
    return numericalFromString<uint64_t, true>(view, isNegative);
  }
  else {
    return numericalFromString<int64_t, false>(view, isNegative);
  }
}

inline bool isIdentifierCharacter(char ch) {
  return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'z')) ||
         ((ch >= 'A') && (ch <= 'Z')) || (ch == '_');
}

[[nodiscard]] Token readIdentifierToken(std::string_view::const_iterator &pos,
                                        std::string_view::const_iterator end) {
  std::string_view::const_iterator beg = pos++;

  while ((pos != end) && isIdentifierCharacter(*pos)) {
    ++pos;
  }

  auto peek = pos;
  while ((peek != end) && (*peek == ' ')) {
    ++peek;
  }
  if ((peek != end) && (*peek == '(')) {
    // identifier followed by bracket => function
    return Token(std::string(beg, pos), TokenType::FunctionName);
  }
  return Token(std::string(beg, pos), TokenType::Variable);
}

[[nodiscard]] Token readStringToken(std::string_view::const_iterator &pos,
                                    std::string_view::const_iterator end) {
  std::string_view::const_iterator beg = ++pos;

  while ((pos != end) && (*pos != '"')) {
    ++pos;
  }

  return Token(std::string(beg, pos++), TokenType::String);
}

[[nodiscard]] Token readOperatorToken(std::string_view::const_iterator &pos,
                                      std::string_view::const_iterator end) {
  static std::unordered_map<char, unsigned> OPERATORS_1 = {
      {'+', static_cast<unsigned>(OperatorType::Add)},
      {'-', static_cast<unsigned>(OperatorType::Subtract)},
      {'*', static_cast<unsigned>(OperatorType::Multiply)},
      {'/', static_cast<unsigned>(OperatorType::Divide)},
      {'%', static_cast<unsigned>(OperatorType::Modulo)},
      {'^', static_cast<unsigned>(OperatorType::Xor)},
      {'<', static_cast<unsigned>(OperatorType::Incomplete) + '<'},
      {'>', static_cast<unsigned>(OperatorType::Incomplete) + '>'},
      {'=', static_cast<unsigned>(OperatorType::Incomplete) + '='},
      {'!', static_cast<unsigned>(OperatorType::Incomplete) + '!'},
      {'&', static_cast<unsigned>(OperatorType::Incomplete) + '&'},
      {'|', static_cast<unsigned>(OperatorType::Incomplete) + '|'},
      {'a', static_cast<unsigned>(OperatorType::Incomplete) + 'a'},
      {'o', static_cast<unsigned>(OperatorType::Incomplete) + 'o'},
      {'?', static_cast<unsigned>(OperatorType::TernaryQ)},
      {':', static_cast<unsigned>(OperatorType::TernaryE)}};

  static std::unordered_map<char, std::unordered_map<char, unsigned>>
      OPERATORS_2 = {
          {'>',
           {
               {'>', static_cast<unsigned>(OperatorType::ShiftRight)},
               {'=', static_cast<unsigned>(OperatorType::GreaterOrEqual)},
               {'_', static_cast<unsigned>(OperatorType::GreaterThan)},
           }},
          {'<',
           {
               {'<', static_cast<unsigned>(OperatorType::ShiftLeft)},
               {'=', static_cast<unsigned>(OperatorType::LessOrEqual)},
               {'_', static_cast<unsigned>(OperatorType::LessThan)},
           }},
          {'!',
           {
               {'=', static_cast<unsigned>(OperatorType::NotEqual)},
               {'_', static_cast<unsigned>(OperatorType::LogicalNot)},
           }},
          {'=',
           {
               {'=', static_cast<unsigned>(OperatorType::Equal)},
           }},
          {'&',
           {
               {'&', static_cast<unsigned>(OperatorType::LogicalAnd)},
               {'_', static_cast<unsigned>(OperatorType::BitwiseAnd)},
           }},
          {'|',
           {
               {'|', static_cast<unsigned>(OperatorType::LogicalOr)},
               {'_', static_cast<unsigned>(OperatorType::BitwiseOr)},
           }},
          {'o',
           {
               {'r', static_cast<unsigned>(OperatorType::LogicalOr)},
           }}};

  auto oldPos = pos;

  unsigned result = 0;

  if (auto iter = OPERATORS_1.find(*pos++); iter != OPERATORS_1.end()) {
    result = iter->second;
    if (result > static_cast<unsigned>(OperatorType::Incomplete)) {
      // in cases like <, the single character could be the operator or it could be << or <= so we
      // peek at the next character(s) - if it is part of a sequence, that is the operator and we consume
      // the next character as well, otherwise we use _ in the map to denote the "single character case"

      auto firstChar = result - static_cast<unsigned>(OperatorType::Incomplete);
      if ((firstChar == 'a') && (*pos == 'n') && (*(pos + 1) == 'd')) {
        // and is currently handled as an exception as it's the only three
        // character operator we support
        result = static_cast<unsigned>(OperatorType::LogicalAnd);
        pos += 2;
      } else {
        const auto &nextDict = OPERATORS_2.at(firstChar);
        char nextChar = *pos;
        auto iter = nextDict.find(nextChar);
        if (iter != nextDict.end()) {
          result = iter->second;
          ++pos;
        } else {
          result = nextDict.at('_');
        }
      }

      // throw std::runtime_error(std::format("operator not supported: {}",
      // *pos));
    }
  }

  if (result == 0) {
    // not an operator, revert parse position
    pos = oldPos;
  }

  return Token(static_cast<OperatorType>(result));
}

[[nodiscard]] Token nextToken(std::string_view::const_iterator &pos,
                              std::string_view::const_iterator end) {
  while ((*pos == ' ') || (*pos == ',')) {
    ++pos;
  }
  char ch = *pos;
  if (((ch >= '0') && (ch <= '9')) || (ch == '-')) {
    return readNumberToken(pos, end);
  } else if (auto tok = readOperatorToken(pos, end); tok.op != OperatorType::Invalid) {
    return tok;
  } else if (isIdentifierCharacter(ch)) {
    return readIdentifierToken(pos, end);
  } else if (ch == '"') {
    return readStringToken(pos, end);
  } else if (ch == '(') {
    ++pos;
    return Token(OperatorType::BracketOpen);
  } else if (ch == ')') {
    ++pos;
    return Token(OperatorType::BracketClose);
  }
  throw std::runtime_error("failed to parse token");
}

void pushBracketClose(std::vector<Token> &output_stack,
                      std::vector<Token> &operator_stack) {
  // while (std::get<OperatorType>((*operator_stack.rbegin()).value) !=
  while (((*operator_stack.rbegin()).op != OperatorType::BracketOpen) &&
         ((*operator_stack.rbegin()).op != OperatorType::ArgumentList)) {
    output_stack.emplace_back(*operator_stack.rbegin());
    operator_stack.pop_back();
  }

  if (operator_stack.rbegin()->op == OperatorType::ArgumentList) {
    output_stack.emplace_back(Token(OperatorType::ArgumentList));
  }

  // remove the bracket itself too
  operator_stack.pop_back();
}

void pushOperator(std::vector<Token> &output_stack,
                  std::vector<Token> &operator_stack, const Token &token) {
  // OperatorType opType = std::get<OperatorType>(token.value);
  OperatorType opType = token.op;
  if (opType == OperatorType::BracketClose) {
    return pushBracketClose(output_stack, operator_stack);
  } else if (opType == OperatorType::BracketOpen) {
    operator_stack.emplace_back(token);
    return;
  }

  int operatorOrder = getOperatorOrder(opType);

  while (!operator_stack.empty() &&
         // (getOperatorOrder(std::get<OperatorType>(
         //      operator_stack.rbegin()->value)) <= operatorOrder)) {
         (getOperatorOrder(operator_stack.rbegin()->op) <= operatorOrder)) {
    auto op = *operator_stack.rbegin();
    operator_stack.pop_back();
    output_stack.emplace_back(op);
  }

  operator_stack.emplace_back(token);
}

void pushToStack(std::vector<Token> &output_stack,
                 std::vector<Token> &operator_stack, const Token &token) {
  if (token.type == TokenType::Operator) {
    pushOperator(output_stack, operator_stack, token);
  } else {
    output_stack.emplace_back(token);
  }
}

std::vector<Token> tokenize(std::string_view input) {
  std::vector<Token> output_stack;
  std::vector<Token> operator_stack;

  std::string_view::const_iterator iter = input.cbegin();

  bool expectArgumentList = false;

  while (iter != input.cend()) {
    Token token = nextToken(iter, input.cend());
    if (expectArgumentList) {
      if ((token.type == TokenType::Operator) &&
          (token.op == OperatorType::BracketOpen)) {
        token.op = OperatorType::ArgumentList;
      } else {
        throw std::runtime_error("expected argument list for function");
      }
    }
    pushToStack(output_stack, operator_stack, token);
    expectArgumentList = token.type == TokenType::FunctionName;
  }

  for (auto iter = operator_stack.rbegin(); iter != operator_stack.rend();
       ++iter) {
    output_stack.push_back(*iter);
  }

  return output_stack;
}

} // namespace SYP
