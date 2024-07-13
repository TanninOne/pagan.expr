#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace SYP {

enum class TokenType {
  Undefined,
  Operator,
  Signed,
  Unsigned,
  Float,
  Numeric,
  Boolean,
  Variable,
  String,
  FunctionName,
  Function,
};

enum class OperatorType : unsigned {
  Invalid,

  // arithmetics
  Add,
  Subtract,

  Multiply,
  Divide,
  Modulo,

  // Bitwise
  ShiftLeft,
  ShiftRight,
  Xor,
  BitwiseAnd,
  BitwiseOr,

  // Comparison
  LessThan,
  LessOrEqual,
  GreaterThan,
  GreaterOrEqual,
  Equal,
  NotEqual,

  // Logical
  LogicalAnd,
  LogicalOr,
  LogicalNot,

  // Special
  TernaryQ,
  TernaryE,

  OperatorCount,

  BracketOpen,
  BracketClose,

  ArgumentList,

  Incomplete,
};

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

/**
 * get priority value (lower value means higher priority) of an operator
 */
int getOperatorOrder(const OperatorType &op);

inline bool operator<(const OperatorType &lhs, const OperatorType &rhs) {
  return getOperatorOrder(lhs) < getOperatorOrder(rhs);
}

struct Token;

using VariableId = uint64_t;
using KnownVariables = std::vector<std::string>;
using DynamicFunction = std::function<Token(const std::vector<Token> &begin)>;
using KnownFunctions = std::vector<std::pair<std::string, DynamicFunction>>;
using TokenValue = std::variant<OperatorType, uint64_t, int64_t, double, bool>;

using TokenQueue = std::vector<Token>;
// using TokenStack = std::stack<Token, std::list<Token>>;         // time 397
// ns, cpu 381 ns, 1723077 iterations using TokenStack = std::stack<Token>; //
// time 322 ns, cpu 289 ns, 2488889 iterations using TokenStack =
// std::stack<Token, std::vector<Token>>;    // time 186 ns, cpu 157 ns, 4480000
// iterations
using TokenStack = std::pair<std::vector<Token>, size_t>;

struct Token {
  using Ptr = std::shared_ptr<Token>;

  TokenType type;
  // TokenValue value;
  union {
    OperatorType op;
    uint64_t unsignedValue;
    int64_t signedValue;
    double floatValue;
    bool boolValue;
  };

  Token() : type(TokenType::Undefined) {}

  Token(const std::string &valueIn, TokenType type) : type(type) {
    if (type == TokenType::FunctionName) {
      initFunction(valueIn);
    } else {
      initVariable(valueIn);
    }
  }

  // need to be able to call with a single argument, otherwise const char* might
  // end up implicitly casted to bool
  Token(const char *valueIn, TokenType type = TokenType::String)
      : Token(std::string(valueIn), type) {}

  Token(uint64_t valueIn) : type(TokenType::Unsigned), unsignedValue(valueIn) {}
  Token(int64_t valueIn) : type(TokenType::Signed), signedValue(valueIn) {}
  Token(int valueIn) : type(TokenType::Signed), signedValue(valueIn) {}
  Token(double valueIn) : type(TokenType::Float), floatValue(valueIn) {}
  Token(bool valueIn) : type(TokenType::Boolean), boolValue(valueIn) {}
  Token(const std::string &name, const DynamicFunction &function)
      : type(TokenType::Function) {
    auto iter = std::find_if(
        s_KnownFunctions.begin(), s_KnownFunctions.end(),
        [&](const std::pair<std::string, DynamicFunction> &iter) -> bool {
          return iter.first == name;
        });
    uint64_t offset;
    if (iter == s_KnownFunctions.end()) {
      // unreferenced function, shouldn't happen when coming from a tokenizer
      // but may occur in synthetic unit tests
      offset = s_KnownFunctions.size();
      s_KnownFunctions.emplace_back(name, function);
    } else {
      if (iter->second == nullptr) {
        iter->second = function;
      }
      offset = iter - s_KnownFunctions.begin();
    }
    unsignedValue = offset;
  }

  Token(OperatorType op) : type(TokenType::Operator), op(op) {}

  [[nodiscard]] Token evaluate(TokenStack &iter) const;

  [[nodiscard]] const std::string &getVariableName() const {
    return type == TokenType::FunctionName
               ? s_KnownFunctions[unsignedValue].first
               : s_KnownVariables[unsignedValue];
  }

  [[nodiscard]] const DynamicFunction &getFunction() const {
    return s_KnownFunctions[unsignedValue].second;
  }

private:
  void initFunction(const std::string &valueIn) {
    auto iter = std::find_if(
        s_KnownFunctions.begin(), s_KnownFunctions.end(),
        [&](const std::pair<std::string, DynamicFunction> &iter) -> bool {
          return iter.first == valueIn;
        });
    uint64_t offset;
    if (iter == s_KnownFunctions.end()) {
      offset = s_KnownFunctions.size();
      s_KnownFunctions.emplace_back(valueIn, nullptr);
    } else {
      offset = iter - s_KnownFunctions.begin();
    }
    unsignedValue = offset;
  }

  void initVariable(const std::string &valueIn) {
    auto iter = std::find_if(
        s_KnownVariables.begin(), s_KnownVariables.end(),
        [&](const std::string &iter) -> bool { return iter == valueIn; });
    uint64_t offset;
    if (iter == s_KnownVariables.end()) {
      offset = s_KnownVariables.size();
      s_KnownVariables.emplace_back(valueIn);
    } else {
      offset = iter - s_KnownVariables.begin();
    }
    unsignedValue = offset;
  }

private:
  static KnownVariables s_KnownVariables;
  static KnownFunctions s_KnownFunctions;
  static uint64_t s_NextVariable;
};

/*
class OperatorBase;
class NumericBase;

class Token : public std::enable_shared_from_this<Token> {

public:
  using Ptr = std::shared_ptr<const Token>;

public:
  virtual ~Token() {}

  [[nodiscard]] virtual TokenType getType() const = 0;

  virtual void
  pushToStack(std::vector<Token::Ptr> &output_stack,
              std::vector<std::shared_ptr<const OperatorBase>> &) const {
    output_stack.emplace_back(shared_from_this());
  }

  virtual void evaluate(std::vector<Token::Ptr>::iterator &from,
                        std::vector<Token::Ptr>::iterator &end) const {
    // nop
  }
};

class OperatorBase : public Token {

public:
  using Ptr = std::shared_ptr<const OperatorBase>;

public:
  OperatorBase(OperatorType type) : m_OperatorType(type) {}

  [[nodiscard]] OperatorType getOperatorType() const { return m_OperatorType; }

  operator OperatorType() const { return m_OperatorType; }

private:
  OperatorType m_OperatorType;
};

class Operator final : public OperatorBase {

public:
  using Ptr = std::shared_ptr<const Operator>;

public:
  Operator(OperatorType type) : OperatorBase(type) {}

  virtual ~Operator() override {}

  [[nodiscard]] virtual TokenType getType() const override {
    return TokenType::Operator;
  }

  virtual void
  pushToStack(std::vector<Token::Ptr> &output_stack,
              std::vector<OperatorBase::Ptr> &operator_stack) const override {
    int operatorOrder = getOperatorOrder(getOperatorType());
    while (!operator_stack.empty() &&
           (getOperatorOrder(**operator_stack.rbegin()) > operatorOrder)) {
      auto op = *operator_stack.rbegin();
      operator_stack.pop_back();
      output_stack.emplace_back(op);
    }
    operator_stack.emplace_back(
        std::static_pointer_cast<const Operator>(shared_from_this()));
  }

  virtual void evaluate(std::vector<Token::Ptr>::iterator &from,
                        std::vector<Token::Ptr>::iterator &end) const override;

private:
  std::shared_ptr<Token>
  sum(const std::vector<Token::Ptr>::iterator &from,
      const std::vector<Token::Ptr>::iterator &end) const;

  std::shared_ptr<Token>
  multiply(const std::vector<Token::Ptr>::iterator &from,
           const std::vector<Token::Ptr>::iterator &end) const;

};

class NumericBase : public Token {
public:
  using Ptr = std::shared_ptr<const NumericBase>;

public:

  virtual NumericBase::Ptr add(const Token::Ptr& rhs) const = 0;
  virtual NumericBase::Ptr multiply(const Token::Ptr& rhs) const = 0;
};

template <arithmetic T>
class Numeric final : public NumericBase {

public:
  using Ptr = std::shared_ptr<const Numeric<T>>;

public:
  Numeric(T value) : m_Value(value) {}

  [[nodiscard]] virtual TokenType getType() const override {
    return TokenType::Numeric;
  }

  T getValue() const { return m_Value; }

  operator T() const { return m_Value; }

  virtual NumericBase::Ptr add(const Token::Ptr& rhs) const override {
    T result = m_Value + std::static_pointer_cast<const
Numeric<T>>(rhs)->m_Value; return std::make_shared<const Numeric<T>>(result);
  }

  virtual NumericBase::Ptr multiply(const Token::Ptr& rhs) const override {
    return std::make_shared<const Numeric<T>>(m_Value *
std::static_pointer_cast<const Numeric<T>>(rhs)->m_Value);
  }

private:
  T m_Value;

};

using Signed = Numeric<int64_t>;
using Unsigned = Numeric<uint64_t>;
using Float = Numeric<float>;

class Boolean final : public Token {

public:
  using Ptr = std::shared_ptr<const Boolean>;

public:
  [[nodiscard]] virtual TokenType getType() const override {
    return TokenType::Boolean;
  }

  operator bool() const { return m_Value; }

private:
  bool m_Value;
};

using Resolver = std::function<float(const std::string &)>;

class Variable final : public Token {

public:
  using Ptr = std::shared_ptr<const Variable>;

public:
  Variable(std::string_view name) : m_Name(name) {}

  [[nodiscard]] virtual TokenType getType() const override {
    return TokenType::Variable;
  }

  [[nodiscard]] float evaluateVariable(const Resolver &resolver) const {
    return resolver(m_Name);
  }

private:
  std::string m_Name;
};

class Bracket final : public OperatorBase {

public:
  using Ptr = std::shared_ptr<const Variable>;

public:
  Bracket(bool open) : OperatorBase(OperatorType::Bracket), m_Open(open) {}

  [[nodiscard]] virtual TokenType getType() const override {
    return TokenType::Bracket;
  }

  virtual void
  pushToStack(std::vector<Token::Ptr> &output_stack,
              std::vector<OperatorBase::Ptr> &operator_stack) const override {
    if (m_Open) {
      operator_stack.push_back(
          std::static_pointer_cast<const Bracket>(shared_from_this()));
    } else {
      while ((*operator_stack.rbegin())->getType() != TokenType::Bracket) {
        auto op = *operator_stack.rbegin();
        operator_stack.pop_back();
        output_stack.emplace_back(op);
      }
      // remove the bracket itself too
      operator_stack.pop_back();
    }
  }

private:
  bool m_Open;
};

*/

} // namespace SYP
