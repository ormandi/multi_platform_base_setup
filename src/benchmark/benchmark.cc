#include "src/benchmark/benchmark.h"

#include <cstdint>
#include <iostream>
#include <random>

#include "absl/container/flat_hash_set.h"
#include "src/benchmark/stdlib_info.h"

namespace benchmark {

void RunBenchmark(int n) {
  std::mt19937_64 rng(0xC0FFEE);
  std::uniform_int_distribution<std::int64_t> dist;

  absl::flat_hash_set<std::int64_t> values;
  values.reserve(n);
  for (int i = 0; i < n; ++i) {
    values.insert(dist(rng));
  }

  std::cout << "arch:   " << ArchName() << "\n"
            << "stdlib: " << StdlibName() << "\n"
            << "values: " << values.size() << " / " << n << "\n";
}

}  // namespace benchmark
