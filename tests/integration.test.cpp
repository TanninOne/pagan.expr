#include "evaluate.h"
#include "shunting_yard.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <iostream>

using namespace std::literals;
using namespace SYP;

Token mystrlen(const std::vector<Token> &input)
{
  std::cout << "mystrlen: " << input.at(0).getVariableName() << std::endl;
  return Token{ input.at(0).getVariableName().length() };
}

Token twoIs2(const std::string &variable) {
  if (variable == "two") {
    return Token(2);
  } else if (variable == "two_double") {
    return Token(2.0);
  } else if (variable == "length") {
    return Token{ "length", mystrlen };
  }

  throw std::runtime_error(std::format("unexpected variable name {}", variable));
}

TEST_CASE("CalculatesSimpleTerm", "[Integration]") {
  REQUIRE(std::get<double>(evaluate(tokenize("3.0 * two_double"), twoIs2)) == 6);
  REQUIRE(std::get<bool>(evaluate(tokenize("3.0 * two_double == 6.0"), twoIs2)));
}

TEST_CASE("CalculatesTrueTerms", "[Integration]") {
  auto token = GENERATE("two + 3 == 5", "two * 3 == 6", "two / 2 == 1",
                        "two % 1 == 0", "two * two == 4", "two + (-3) == -1",
                        "(two + 4) / 2 == 3", "1 || 0", "1 or 0", "!(2 == 4)");
  REQUIRE(std::get<bool>(evaluate(tokenize(token), twoIs2)));
}

TEST_CASE("CalculatesFalseTerms", "[Integration]") {
  auto token = GENERATE("1 && 0", "1 and 0", "!(two == 2)");
  REQUIRE_FALSE(std::get<bool>(evaluate(tokenize(token), twoIs2)));
}

TEST_CASE("CalculatesStrTerms", "[Integration]") {
  auto [term, res] = GENERATE(std::make_pair("\"foobar\"", "foobar"),
                              std::make_pair("\"foo\" + \"bar\"", "foobar"));
  REQUIRE(std::get<std::string>(evaluate(tokenize(term), twoIs2)) == res);
}

TEST_CASE("CalculatesNumericTerms", "[Integration]") {
  auto [term, res] =
      GENERATE(std::make_pair("two ^ 7", 5), std::make_pair("0xFF & two", 2),
               std::make_pair("two << 2", 8), std::make_pair("two >> 1", 1),
               std::make_pair("length(\"foobar\")", 6),
               // extra arguments are ignored
               std::make_pair("length(\"foobar\", \"foo\")", 6),
               // operations in argument list are evaluated
               std::make_pair("length(\"foo\" + \"bar\")", 6),
               // extra brackets in argument list are evaluated
               std::make_pair("length((\"foo\" + \"bar\"))", 6)
               );
  std::cout << "numeric terms: " << term << std::endl;
  REQUIRE(std::get<uint64_t>(evaluate(tokenize(term), twoIs2)) == res);
}

TEST_CASE("CalculatesTernaryTerms", "[Integration]") {
  auto [term, res] = GENERATE(
      std::make_pair("(two == 2) ? 42 : 666", 42),
      std::make_pair("(two == 2) ? (21 * 2) : (22 * 30 + 6)", 42),
      std::make_pair("5 + ((two == 2) ? (21 * 2) : (22 * 30 + 6))", 47));
  REQUIRE(std::get<int64_t>(evaluate(tokenize(term), twoIs2)) == res);
}
