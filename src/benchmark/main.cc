#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "src/benchmark/benchmark.h"

ABSL_FLAG(int, n, 1000000, "Number of random integers to generate.");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  benchmark::RunBenchmark(absl::GetFlag(FLAGS_n));
  return 0;
}
