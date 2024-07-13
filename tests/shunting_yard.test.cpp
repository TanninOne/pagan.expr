#include <gtest/gtest.h>
#include <memory>

#include "../src/shunting_yard.h"

using namespace std::literals;

TEST(ShuntingYard, ParsesTwoOperandStatement) {
  auto tokens = tokenize("1 + 1"sv);

  EXPECT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[2].type, TokenType::Operator);
  EXPECT_EQ(tokens[2].op, OperatorType::Add);
}

TEST(ShuntingYard, ParsesThreeOperandStatement) {
  auto tokens = tokenize("1 + 1 + 1"sv);

  EXPECT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[2].type, TokenType::Operator);
  EXPECT_EQ(tokens[4].type, TokenType::Operator);
  EXPECT_EQ(tokens[2].op, OperatorType::Add);
  EXPECT_EQ(tokens[4].op, OperatorType::Add);
}

TEST(ShuntingYard, ObeysOperatorPriority) {
  auto tokens = tokenize("1 + 2 * 2"sv);

  EXPECT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[3].type, TokenType::Operator);
  EXPECT_EQ(tokens[4].type, TokenType::Operator);
  EXPECT_EQ(tokens[3].op, OperatorType::Multiply);
  EXPECT_EQ(tokens[4].op, OperatorType::Add);
}

TEST(ShuntingYard, PairsOperatorWithArguments) {
  auto tokens = tokenize("2 * 2 + 1"sv);

  EXPECT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[2].type, TokenType::Operator);
  EXPECT_EQ(tokens[4].type, TokenType::Operator);
  EXPECT_EQ(tokens[2].op, OperatorType::Multiply);
  EXPECT_EQ(tokens[4].op, OperatorType::Add);
}

TEST(ShuntingYard, SupportsBrackets) {
  auto tokens = tokenize("(1 + 1) * 2");

  EXPECT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[2].type, TokenType::Operator);
  EXPECT_EQ(tokens[4].type, TokenType::Operator);
  EXPECT_EQ(tokens[2].op, OperatorType::Add);
  EXPECT_EQ(tokens[4].op, OperatorType::Multiply);
}

TEST(ShuntingYard, SupportsComparison) {
  auto tokens = tokenize("1 + 1 == 2"sv);

  EXPECT_EQ(tokens.size(), 5);
  EXPECT_EQ(tokens[2].type, TokenType::Operator);
  EXPECT_EQ(tokens[4].type, TokenType::Operator);
  EXPECT_EQ(tokens[2].op, OperatorType::Add);
  EXPECT_EQ(tokens[4].op, OperatorType::Equal);
}

TEST(ShuntingYard, SupportsVariables) {
  auto tokens = tokenize("var + 5 < 12"sv);

  EXPECT_EQ(tokens[0].type, TokenType::Variable);
  EXPECT_EQ(tokens[0].getVariableName(), "var");
}

class ShuntingYardTerms : public testing::TestWithParam<std::string_view> {};

TEST_P(ShuntingYardTerms, SupportsVariousSimpleTerms) {
  auto tokens = tokenize(GetParam());

  EXPECT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[2].type, TokenType::Operator);
}

INSTANTIATE_TEST_SUITE_P(SimpleTerms, ShuntingYardTerms,
                         testing::Values("2 * 2", "5 % 2"));
