#include "../src/evaluate.h"
#include "../src/shunting_yard.h"

#include <gtest/gtest.h>
#include <memory>
#include <variant>

using namespace std::literals;

Token mystrlen(const std::vector<Token> &input)
{
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

  throw std::runtime_error("unexpected variable name");
}

TEST(Integration, CalculatesSimpleTerm) {
  EXPECT_EQ(std::get<double>(evaluate(tokenize("3.0 * two_double"), twoIs2)), 6);
  EXPECT_TRUE(std::get<bool>(evaluate(tokenize("3.0 * two_double == 6.0"), twoIs2)));
}

class IntegrationTrueTerms : public testing::TestWithParam<std::string_view> {};
class IntegrationFalseTerms : public testing::TestWithParam<std::string_view> {};

class IntegrationEqTermsStr : public testing::TestWithParam<std::pair<std::string_view, std::string>> {};
class IntegrationEqTermsNumeric : public testing::TestWithParam<std::pair<std::string_view, uint64_t>> {};
class IntegrationEqTermsTernary : public testing::TestWithParam<std::pair<std::string_view, uint64_t>> {};

TEST_P(IntegrationTrueTerms, CalculatesTrueTerms) {
  EXPECT_TRUE(std::get<bool>(evaluate(tokenize(GetParam()), twoIs2)));
}

TEST_P(IntegrationFalseTerms, CalculatesFalseTerms) {
  EXPECT_FALSE(std::get<bool>(evaluate(tokenize(GetParam()), twoIs2)));
}

TEST_P(IntegrationEqTermsStr, CalculatesStrTerms) {
  auto [term, res] = GetParam();
  EXPECT_EQ(std::get<std::string>(evaluate(tokenize(term), twoIs2)), res);
}

TEST_P(IntegrationEqTermsNumeric, CalculatesNumericTerms) {
  auto [term, res] = GetParam();
  EXPECT_EQ(std::get<uint64_t>(evaluate(tokenize(term), twoIs2)), res);
}

TEST_P(IntegrationEqTermsTernary, CalculatesTernaryTerms) {
  auto [term, res] = GetParam();
  EXPECT_EQ(std::get<int64_t>(evaluate(tokenize(term), twoIs2)), res);
}

INSTANTIATE_TEST_SUITE_P(TrueTerms, IntegrationTrueTerms,
                         testing::Values(
                         "two + 3 == 5",
                         "two * 3 == 6",
                         "two / 2 == 1",
                         "two % 1 == 0",
                         "two * two == 4",
                         "two + (-3) == -1",
                         "(two + 4) / 2 == 3",
                         "1 || 0",
                         "1 or 0",
                         "!(2 == 4)"
                         ));

INSTANTIATE_TEST_SUITE_P(FalseTerms, IntegrationFalseTerms,
                         testing::Values(
                         "1 && 0",
                         "1 and 0",
                         "!(two == 2)"
                         ));

INSTANTIATE_TEST_SUITE_P(EqStrTerms, IntegrationEqTermsStr,
                         testing::Values(std::make_pair("\"foobar\"", "foobar"),
                                         std::make_pair("\"foo\" + \"bar\"",
                                                        "foobar")));

INSTANTIATE_TEST_SUITE_P(EqNumericTerms, IntegrationEqTermsNumeric,
                         testing::Values(std::make_pair("two ^ 7", 5),
                                         std::make_pair("0xFF & two", 2),
                                         std::make_pair("two << 2", 8),
                                         std::make_pair("two >> 1", 1),
                                         std::make_pair("length(\"foobar\")", 6),
                                         // extra arguments are ignored
                                         std::make_pair("length(\"foobar\", \"foo\")", 6),
                                         // operations in argument list are evaluated
                                         std::make_pair("length(\"foo\" + \"bar\")", 6),
                                         // extra brackets in argument list are evaluated
                                         std::make_pair("length((\"foo\" + \"bar\"))", 6)
                                         ));

INSTANTIATE_TEST_SUITE_P(
    EqTernaryTerms, IntegrationEqTermsTernary,
    testing::Values(std::make_pair("(two == 2) ? 42 : 666", 42),
                    std::make_pair("(two == 2) ? (21 * 2) : (22 * 30 + 6)", 42),
                    std::make_pair("5 + ((two == 2) ? (21 * 2) : (22 * 30 + 6))",
                                   47)));

/*


// ternary
REQUIRE(makeFunc<int>("(x == 2) ? 42 : 666")(query) == 42);
REQUIRE(makeFunc<int>("(x == 2) ? (21 * 2) : (22 * 30 + 6)")(query) == 42);
REQUIRE(makeFunc<int>("5 + ((x == 2) ? (21 * 2) : (22 * 30 + 6))")(query) ==
        47);
// complex term from .rar
REQUIRE(makeFunc<std::string>(
            "(year <= 999 ? (\"0\" + (year <= 99 ? (\"0\" + (year <= 9 ? \"0\" "
            ": \"\")) : \"\")) : \"\") + year.to_s")(query) == "0002");
// same term with line breaks
REQUIRE(makeFunc<std::string>(
            "(year <= 999 ? (\"0\" +\r\n  (year <= 99 ? (\"0\" +\r\n    (year "
            "<= 9 ? \"0\" : \"\")\r\n  ) : \"\")\r\n) : \"\") + year.to_s")(
            query) == "0002");
*/
