#include "../src/shunting_yard.h"

#include <string_view>
#include <benchmark/benchmark.h>

using namespace std::string_view_literals;

static void BM_ParseTerm(benchmark::State& state) {
  std:: string expression = "3 * two == 6";
  for (auto _ : state)
  {
    auto tokens = tokenize(expression);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_ParseTerm);
