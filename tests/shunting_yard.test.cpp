#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <memory>

#include "shunting_yard.h"

using namespace std::literals;
using namespace SYP;

TEST_CASE("parses two operand statement", "[ShuntingYard]") {
  auto tokens = tokenize("1 + 1"sv);

  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[2].type == TokenType::Operator);
  REQUIRE(tokens[2].op == OperatorType::Add);
}

TEST_CASE("parses three operand statement", "[ShuntingYard]") {
  auto tokens = tokenize("1 + 1 + 1"sv);

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[2].type == TokenType::Operator);
  REQUIRE(tokens[4].type == TokenType::Operator);
  REQUIRE(tokens[2].op == OperatorType::Add);
  REQUIRE(tokens[4].op == OperatorType::Add);
}

TEST_CASE("obeys operator priority", "[ShuntingYard]") {
  auto tokens = tokenize("1 + 2 * 2"sv);

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[3].type == TokenType::Operator);
  REQUIRE(tokens[4].type == TokenType::Operator);
  REQUIRE(tokens[3].op == OperatorType::Multiply);
  REQUIRE(tokens[4].op == OperatorType::Add);
}

TEST_CASE("pairs operator with arguments", "[ShuntingYard]") {
  auto tokens = tokenize("2 * 2 + 1"sv);

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[2].type == TokenType::Operator);
  REQUIRE(tokens[4].type == TokenType::Operator);
  REQUIRE(tokens[2].op == OperatorType::Multiply);
  REQUIRE(tokens[4].op == OperatorType::Add);
}

TEST_CASE("supports brackets", "[ShuntingYard]") {
  auto tokens = tokenize("(1 + 1) * 2");

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[2].type == TokenType::Operator);
  REQUIRE(tokens[4].type == TokenType::Operator);
  REQUIRE(tokens[2].op == OperatorType::Add);
  REQUIRE(tokens[4].op == OperatorType::Multiply);
}

TEST_CASE("supports comparison", "[ShuntingYard]") {
  auto tokens = tokenize("1 + 1 == 2"sv);

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[2].type == TokenType::Operator);
  REQUIRE(tokens[4].type == TokenType::Operator);
  REQUIRE(tokens[2].op == OperatorType::Add);
  REQUIRE(tokens[4].op == OperatorType::Equal);
}

TEST_CASE("supports variables", "[ShuntingYard]") {
  auto tokens = tokenize("var + 5 < 12"sv);

  REQUIRE(tokens[0].type == TokenType::Variable);
  REQUIRE(tokens[0].getVariableName() == "var");
}

TEST_CASE("supports various simple terms", "[ShuntingYardTerms]") {
  auto token = GENERATE("2 * 2", "5 % 2");
  auto tokens = tokenize(token);

  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[2].type == TokenType::Operator);
}
