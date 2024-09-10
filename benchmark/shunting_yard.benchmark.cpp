#include "shunting_yard.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

TEST_CASE("benchmark shunting_yard", "[ShuntingYard]") {
  std:: string expression = "3 * two == 6";
  BENCHMARK("tokenize") {
    return SYP::tokenize(expression);
  };
}
