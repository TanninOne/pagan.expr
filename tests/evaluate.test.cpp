#include <catch2/catch_test_macros.hpp>

#include "evaluate.h"

using namespace std::literals;
using namespace SYP;

TEST_CASE("SupportsAddition", "[Evalute]") {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(OperatorType::Add),
  };

  REQUIRE(std::get<int64_t>(evaluate(tokens)) == 2);
}

TEST_CASE("SupportsMultipleOperations", "[Evalute]") {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(1),
    Token(OperatorType::Add),
    Token(OperatorType::Add),
  };

  REQUIRE(std::get<int64_t>(evaluate(tokens)) == 3);
}

TEST_CASE("SupportsMultiplication", "[Evalute]") {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(OperatorType::Add),
    Token(2),
    Token(OperatorType::Multiply),
  };

  REQUIRE(std::get<int64_t>(evaluate(tokens)) == 4);
}

TEST_CASE("SupportsComparison", "[Evalute]") {
  std::vector<Token> tokens {
    Token(1),
    Token(1),
    Token(OperatorType::Add),
    Token(2),
    Token(OperatorType::Equal),
  };

  REQUIRE(std::get<bool>(evaluate(tokens)) == true);

  tokens[3].signedValue = 3;
  REQUIRE(std::get<bool>(evaluate(tokens)) == false);
}

TEST_CASE("SupportsFloat", "[Evalute]") {
  std::vector<Token> tokens {
    Token(3.0f),
    Token(0.5f),
    Token(OperatorType::Multiply),
  };

  REQUIRE(std::get<double>(evaluate(tokens)) == 1.5f);
}

Token varIsTwo(const std::string &variable) {
  if (variable == "var") {
    return Token(2.0);
  }

  throw std::runtime_error("unexpected variable name");
}

TEST_CASE("SupportsVariables", "[Evalute]") {
  std::vector<Token> tokens {
    Token{ "var", TokenType::Variable },
    Token{ 2.0 },
    Token{ OperatorType::Multiply },
  };

  REQUIRE(std::get<double>(evaluate(tokens, varIsTwo)) == 4);
}

TEST_CASE("Performance", "[Evalute]") {
  std::vector<Token> tokens {
    Token{ "var", TokenType::Variable },
    Token{ 2.0 },
    Token{ OperatorType::Multiply },
  };

  for (int i = 0; i < 1000000; i++) {
    REQUIRE(std::get<double>(evaluate(tokens, varIsTwo)) == 4.0);
  }
}

