# multi_platform_base_setup

A minimal Bazel base project that builds C++20 with a **fully hermetic,
zero-sysroot LLVM toolchain** ([hermeticbuild/hermetic-llvm], published to the
Bazel Central Registry as the `llvm` module). It cross-compiles to five
platform / C++ standard library combinations and demonstrates a
**manylinux-compatible** variant that is always built against GNU `libstdc++`.

The toolchain compiles every target-specific component (CRT, libc, libc++ /
libstdc++, compiler-rt, libunwind) from source, so no host compiler or sysroot
is needed and builds are reproducible across machines.

## Prerequisites

- [`bazelisk`](https://github.com/bazelbuild/bazelisk) on `PATH`. It reads
  `.bazelversion` and pins **Bazel 9.1.1** automatically.

Nothing else — the C/C++ toolchain is downloaded and built hermetically by Bazel.

## Pinned versions (reproducibility)

Every version is fixed in-tree and only changes by a manual edit. Nothing floats
to "latest".

| Component | Version | Defined in |
|---|---|---|
| Bazel | `9.1.1` | `.bazelversion` |
| `llvm` toolchain module | `0.8.9` | `MODULE.bazel` |
| LLVM / `libc++` | `21.1.8` | `MODULE.bazel` (`llvm_source.version`) |
| `libstdc++` (GNU/gcc line) | `17.0.0` | `platforms/BUILD.bazel` (`libstdcxx.17.0.0`) |
| glibc | `2.28` | `platforms/BUILD.bazel` (`libc:gnu.2.28`) |
| abseil-cpp | `20260526.0` | `MODULE.bazel` |

> The `llvm` module 0.8.9 ships LLVM 21.1.8 / 22.1.x only, so 21.1.8 is the
> pinned compiler (it has prebuilt archives, so no slow from-source bootstrap).

## Platforms

`clang` is always the compiler; the "stdc++ (GNU)" platforms only change which
C++ *runtime* (`libstdc++`, built from the gcc-17 sources) is linked.

| Platform (`//platforms:...`) | OS / CPU | C++ stdlib |
|---|---|---|
| `macos_arm64_libcxx` | macOS / arm64 | libc++ (LLVM) |
| `linux_arm64_libcxx` | Linux / arm64 | libc++ (LLVM) |
| `linux_arm64_stdcxx` | Linux / arm64 | libstdc++ (GNU) |
| `linux_x86_64_libcxx` | Linux / x86_64 | libc++ (LLVM) |
| `linux_x86_64_stdcxx` | Linux / x86_64 | libstdc++ (GNU) |

Each has a matching `.bazelrc` shortcut, e.g. `--config=linux_x86_64_libcxx`.

## Execution platforms

The **target platform** (above) is what the artifact runs on. The **execution
platform** is the host running Bazel. Because the toolchain is fully hermetic and
cross-compiles every target component from source, **any** supported execution
platform can build **all five** target platforms — the `--config`/`--platforms`
choice is independent of the host.

Common to every host: `bazelisk` on `PATH` and network access on the first build
(to fetch prebuilt LLVM and the runtime sources). The first build of a given
target is slow because libc++/libstdc++/compiler-rt/libunwind are compiled from
source; afterwards everything is cached.

| Execution platform (host) | Builds all 5 targets | Expectations / extra requirements |
|---|---|---|
| macOS / arm64 | yes (cross) | For `stdcxx` (GNU libstdc++) targets, a bash ≥ 4.4 is required — Apple's `/bin/bash` is 3.2 and fails the libstdc++ probe scripts. `.bazelrc` sets this automatically via `build:macos --shell_executable=...` (see note below). Targeting macOS downloads the Apple SDK hermetically from Apple's CDN. |
| macOS / x86_64 | yes (cross) | Same as macOS / arm64 (bash ≥ 4.4 for `stdcxx`; macOS SDK downloaded hermetically). |
| Linux / x86_64 | yes | No extra requirements — distro bash is ≥ 4.4. Builds and runs the native `linux_x86_64_*` artifacts directly. |
| Linux / arm64 | yes | No extra requirements. Builds and runs the native `linux_arm64_*` artifacts directly. |

A cross-built binary only **executes** on a host matching its *target* platform
(e.g. a `linux_x86_64_stdcxx` binary runs on Linux x86_64, not on the macOS host
that built it). Building it cross-platform always works; running it does not.

## Build and run

The example binary `//src/benchmark:benchmark` inserts `--n` random integers
into an `absl::flat_hash_set` and prints the target architecture and the C++
standard library in use. It is linked against **libc++ (LLVM)**.

```sh
# Build everything for the host (macOS arm64 -> libc++).
bazel build //...

# Run the example; --n comes from an absl flag.
bazel run //src/benchmark:benchmark -- --n=5000000
# arch:   arm64
# stdlib: libc++ (LLVM)
# values: 4999... / 5000000
```

## Cross-compiling

Select a target platform on the command line; the predefined artifact for that
platform is produced hermetically (no host toolchain involved):

```sh
bazel build --config=linux_x86_64_libcxx //src/benchmark:benchmark
bazel build --config=linux_arm64_libcxx  //src/benchmark:benchmark
```

The `benchmark` binary is restricted to libc++ platforms, so under a `stdcxx`
platform it is reported incompatible and skipped — that is intentional.

## manylinux (stdc++ / GNU)

`manylinux` wheels require glibc + `libstdc++`, not `libc++`. The boolean config
parameter `//:manylinux_compatible` controls this:

- `//src/benchmark:benchmark_manylinux` is a `cc_library` that **reuses the same
  core code** (`:benchmark_lib`) but is forced to **stdc++ (GNU)** on Linux via a
  Starlark transition whenever `--//:manylinux_compatible=True`
  (`--config=manylinux`).
- `//src/benchmark:benchmark_manylinux_bin` is a runnable proof, built under a
  Linux `stdcxx` platform.

```sh
# Build the manylinux-compatible library (forced to stdc++).
bazel build --config=manylinux --config=linux_x86_64_stdcxx \
    //src/benchmark:benchmark_manylinux

# Build the stdc++ binary (a Linux x86_64 ELF, dynamically linked to libstdc++).
bazel build --config=linux_x86_64_stdcxx //src/benchmark:benchmark_manylinux_bin
# Run it on a Linux x86_64 host (cross-built binaries execute on their target OS):
#   ./bazel-bin/src/benchmark/benchmark_manylinux_bin
#   arch:   x86_64
#   stdlib: libstdc++ (GNU)
```

> `libstdc++` is supported only as a **dynamic** runtime on Linux/glibc, so the
> stdc++ binary sets `linkstatic = False`. Do not pass `--dynamic_mode=off`.
>
> **macOS hosts:** building the stdc++ targets runs toolchain probe scripts that
> need bash ≥ 4.4, but Apple ships bash 3.2 at `/bin/bash`. `.bazelrc` points
> Bazel at a newer bash via `build:macos --shell_executable=/opt/homebrew/bin/bash`
> (applied automatically on macOS). Adjust that path if Homebrew is elsewhere.
> The libc++ targets and Linux hosts do not need this.

## Layout

```
MODULE.bazel              # toolchain + deps, pinned versions
.bazelrc                  # C++20 + per-platform / manylinux configs
BUILD.bazel               # manylinux_compatible bool_flag + config_setting
platforms/BUILD.bazel     # the 5 target platforms
src/benchmark/
  stdlib_info.{h,cc}      # arch + C++ stdlib detection (reused)
  benchmark.{h,cc}        # core: flat_hash_set benchmark (reused)
  main.cc                 # entry point; --n absl flag
  transitions.bzl         # manylinux stdc++ transition + cc_library wrapper
  BUILD.bazel
```

[hermeticbuild/hermetic-llvm]: https://github.com/hermeticbuild/hermetic-llvm
