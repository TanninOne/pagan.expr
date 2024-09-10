#include "../src/evaluate.h"
#include "../src/shunting_yard.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <cstdint>
#include <stdexcept>

using namespace std::string_view_literals;
using namespace SYP;

const Token& variables(const std::string &name) {
  static std::unordered_map<std::string, Token> variables = {
    {"two", Token(2)}
  };
  auto iter = variables.find(name);
  if (iter != variables.end())
  {
    return iter->second;
  }
  throw std::runtime_error("Unknown variable: " + name);
}

TEST_CASE("benchmark evaluate", "[Evaluate]") {
  auto tokens = tokenize("3 * two == 6"sv);
  BENCHMARK("evaluate") {
    return evaluate(tokens, variables);
  };

  int64_t var = 2;
  BENCHMARK("evaluate reference") {
    return (3 * var) == 6;
  };
}

