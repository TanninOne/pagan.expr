#include "../src/evaluate.h"
#include "../src/shunting_yard.h"

#include <benchmark/benchmark.h>
#include <stdexcept>
#include <string_view>
#include <iostream>

using namespace std::string_view_literals;

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

static void BM_EvaluateTerm(benchmark::State& state) {
  auto tokens = tokenize("3 * two == 6"sv);
  for (auto _ : state)
  {
    auto result = evaluate(tokens, variables);
  }
}

BENCHMARK(BM_EvaluateTerm);

static void BM_EvaluateReference(benchmark::State& state) {
  auto tokens = tokenize("3 * two == 6"sv);
  int64_t var = 2;
  for (auto _ : state)
  {
    // volatile so the compiler can't optimize this away
    volatile auto result = (3 * var) == 6;
  }
}

BENCHMARK(BM_EvaluateReference);
