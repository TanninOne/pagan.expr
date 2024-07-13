#include <gtest/gtest.h>

#include "../src/evaluate.h"

using namespace std::literals;

TEST(Evalute, SupportsAddition) {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(OperatorType::Add),
  };

  EXPECT_EQ(std::get<int64_t>(evaluate(tokens)), 2);
}

TEST(Evalute, SupportsMultipleOperations) {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(1),
    Token(OperatorType::Add),
    Token(OperatorType::Add),
  };

  EXPECT_EQ(std::get<int64_t>(evaluate(tokens)), 3);
}

TEST(Evalute, SupportsMultiplication) {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(OperatorType::Add),
    Token(2),
    Token(OperatorType::Multiply),
  };

  EXPECT_EQ(std::get<int64_t>(evaluate(tokens)), 4);
}

TEST(Evalute, SupportsComparison) {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(OperatorType::Add),
    Token(2),
    Token(OperatorType::Equal),
  };

  EXPECT_EQ(std::get<bool>(evaluate(tokens)), true);

  tokens[3].signedValue = 3;
  EXPECT_EQ(std::get<bool>(evaluate(tokens)), false);
}

TEST(Evalute, SupportsFloat) {
  std::vector<Token> tokens {
    Token(3.0f),
    Token(0.5f),
    Token(OperatorType::Multiply),
  };

  EXPECT_EQ(std::get<double>(evaluate(tokens)), 1.5f);
}

Token varIsTwo(const std::string &variable) {
  if (variable == "var") {
    return Token(2.0);
  }

  throw std::runtime_error("unexpected variable name");
}

TEST(Evalute, SupportsVariables) {
  std::vector<Token> tokens {
    Token{ "var", TokenType::Variable },
    Token{ 2.0 },
    Token{ OperatorType::Multiply },
  };

  EXPECT_EQ(std::get<double>(evaluate(tokens, varIsTwo)), 4);
}

TEST(Evalute, Performance) {
  std::vector<Token> tokens {
    Token{ "var", TokenType::Variable },
    Token{ 2.0 },
    Token{ OperatorType::Multiply },
  };

  for (int i = 0; i < 1000000; i++) {
    EXPECT_EQ(std::get<double>(evaluate(tokens, varIsTwo)), 4.0);
  }
}

