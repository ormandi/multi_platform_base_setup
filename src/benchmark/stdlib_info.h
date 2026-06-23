#ifndef SRC_BENCHMARK_STDLIB_INFO_H_
#define SRC_BENCHMARK_STDLIB_INFO_H_

#include <string>

namespace benchmark {

// Returns the CPU architecture the binary was compiled for.
std::string ArchName();

// Returns the C++ standard library the binary is linked against.
std::string StdlibName();

}  // namespace benchmark

#endif  // SRC_BENCHMARK_STDLIB_INFO_H_
