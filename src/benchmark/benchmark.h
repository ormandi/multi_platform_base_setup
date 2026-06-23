#ifndef SRC_BENCHMARK_BENCHMARK_H_
#define SRC_BENCHMARK_BENCHMARK_H_

namespace benchmark {

// Inserts `n` random integers into an absl::flat_hash_set and prints the
// resulting size along with the target architecture and C++ standard library.
void RunBenchmark(int n);

}  // namespace benchmark

#endif  // SRC_BENCHMARK_BENCHMARK_H_
