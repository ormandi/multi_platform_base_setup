#include "src/benchmark/stdlib_info.h"

#include <version>  // Populates _LIBCPP_VERSION / __GLIBCXX__.

#include <string>

namespace benchmark {

std::string ArchName() {
#if defined(__aarch64__)
  return "arm64";
#elif defined(__x86_64__)
  return "x86_64";
#else
  return "unknown";
#endif
}

std::string StdlibName() {
#if defined(_LIBCPP_VERSION)
  return "libc++ (LLVM)";
#elif defined(__GLIBCXX__)
  return "libstdc++ (GNU)";
#else
  return "unknown";
#endif
}

}  // namespace benchmark
