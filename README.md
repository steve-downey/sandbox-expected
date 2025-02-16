<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# beman.exemplar: A Beman Library Exemplar

![Library Status](https://github.com/bemanproject/beman/blob/c6997986557ec6dda98acbdf502082cdf7335526/images/badges/beman_badge-beman_library_under_development.svg)
![Continuous Integration Tests](https://github.com/bemanproject/exemplar/actions/workflows/ci_tests.yml/badge.svg)
![Lint Check (pre-commit)](https://github.com/bemanproject/exemplar/actions/workflows/pre-commit.yml/badge.svg)

`beman.exemplar` is a minimal C++ library conforming to [The Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md).
This can be used as a template for those intending to write Beman libraries.
It may also find use as a minimal and modern  C++ project structure.

**Implements**: `std::identity` proposed in [Standard Library Concepts (P0898R3)](https://wg21.link/P0898R3).

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_LIBRARY_MATURITY_MODEL.md#under-development-and-not-yet-ready-for-production-use)

## Usage

`std::identity` is a function object type whose `operator()` returns its argument unchanged.
`std::identity` serves as the default projection in constrained algorithms.
Its direct usage is usually not needed.

### Usage: default projection in constrained algorithms

The following code snippet illustrates how we can achieve a default projection using `beman::exemplar::identity`:

```cpp
#include <beman/exemplar/identity.hpp>

namespace exe = beman::exemplar;

// Class with a pair of values.
struct Pair
{
    int n;
    std::string s;

    // Output the pair in the form {n, s}.
    // Used by the range-printer if no custom projection is provided (default: identity projection).
    friend std::ostream &operator<<(std::ostream &os, const Pair &p)
    {
        return os << "Pair" << '{' << p.n << ", " << p.s << '}';
    }
};

// A range-printer that can print projected (modified) elements of a range.
// All the elements of the range are printed in the form {element1, element2, ...}.
// e.g., pairs with identity: Pair{1, one}, Pair{2, two}, Pair{3, three}
// e.g., pairs with custom projection: {1:one, 2:two, 3:three}
template <std::ranges::input_range R,
          typename Projection>
void print(const std::string_view rem, R &&range, Projection projection = exe::identity>)
{
    std::cout << rem << '{';
    std::ranges::for_each(
        range,
        [O = 0](const auto &o) mutable
        { std::cout << (O++ ? ", " : "") << o; },
        projection);
    std::cout << "}\n";
};

int main()
{
    // A vector of pairs to print.
    const std::vector<Pair> pairs = {
        {1, "one"},
        {2, "two"},
        {3, "three"},
    };

    // Print the pairs using the default projection.
    print("\tpairs with beman: ", pairs);

    return 0;
}

```

Full runnable examples can be found in [`examples/`](examples/).

## Dependency

### Build Environment

This project requires minimal **C++17** and **CMake 3.25** to build.

This project pulls [Google Test](https://github.com/google/googletest)
from GitHub as a development dependency for its testing framework,
thus requiring an active internet connection to configure.
You can disable this behavior by setting cmake option
[`BEMAN_EXEMPLAR_BUILD_TESTS`](#beman_exemplar_build_tests) to `OFF`
when configuring the project.

However,
some examples and tests will not be compiled
unless provided compiler support **C++20** or ranges capabilities enabled.

> [!TIP]
>
> You will be able to see if there's any examples that isn't enabled due to
> compiler capabilities or minimum C++ version it is configured to in the logs.
>
> Below is an example:
>
> ```txt
> -- Looking for __cpp_lib_ranges
> -- Looking for __cpp_lib_ranges - not found
> CMake Warning at examples/CMakeLists.txt:12 (message):
>   Missing range support! Skip: identity_as_default_projection
>
>
> Examples to be built: identity_direct_usage
> ```

### Supported Platforms

This project officially supports:

- GNU GCC Compiler \[version 12-14\]
- LLVM Clang++ Compiler \[version 17-20\]
- AppleClang compiler on Mac OS
- MSVC compiler on Windows

> [!NOTE]
>
> Versions outside of this range would likely work as well,
> especially if you're using a version above the given range
> (e.g. HEAD/ nightly).
> These development environments are verified using our CI configuration.

## Development

### Develop using GitHub Codespace

This project supports [GitHub Codespace](https://github.com/features/codespaces)
via [Development Containers](https://containers.dev/),
which allows rapid development and instant hacking in your browser.
We recommand you using GitHub codespace to explore this project as this
requires minimal setup.

You can create a codespace for this project by clicking this badge:

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/bemanproject/exemplar)

For more detailed documentation regarding creating and developing inside of
GitHub codespaces, please reference [this doc](https://docs.github.com/en/codespaces/).

> [!NOTE]
>
> The codespace container may take up to 5 minutes to build and spin-up,
> this is normal as we need to build a custom docker container to setup
> an environment appropriate for beman projects.

### Develop locally on your machines

<details>
<summary> For Linux based systems </summary>

Beman libraries requires [recent versions of CMake](#build-environment),
we advice you download CMake directly from [CMake's website](https://cmake.org/download/)
or install via the [Kitware apt library](https://apt.kitware.com/).

A [supported compiler](#supported-platforms) should be available from your package manager.
Alternatively you could use an install script from official compiler venders.

Here is an example of how to install the latest stable version of clang
as per [the official LLVM install guide](https://apt.llvm.org/).

```bash
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```

</details>

<details>
<summary> For MacOS based systems </summary>

Beman libraries requires [recent versions of CMake](#build-environment),
you can use `Homebrew` to install the latest major version of CMake.

```bash
brew install cmake
```

A [supported compiler](#supported-platforms) is also available from brew.

For example, you can install latest major release of Clang++ compiler as:

```bash
brew install llvm
```

</details>

### Configure and Build the project using CMake Preset

This project recommands using [CMake Preset](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
to configure, build and test the project.
Appropriate presets for major compilers has been included by default.
You can use `cmake --list-presets` to see all available presets.

Here is an example to invoke the `gcc-debug` preset.

```shell
cmake --workflow --preset gcc-debug
```

Generally, there's two kinds of presets, `debug` and `release`.

The `debug` presets are designed to aid development,
thus it has as much sanitizers turned on as possible.

> [!NOTE]
>
> The set of sanitizer supports are different across compilers,
> you can checout the exact set compiler arguments by looking at the toolchain
> files under the [`cmake`](cmake/) directory.

The `release` presets are designed for use in production environments,
thus it has the highest optimization turned on (e.g. `O3`).

### Configure and Build the project manually

While [CMake Preset](#configure-and-build-the-project-using-cmake-preset) is
convient,
you might want to pass extra config/ compiler arguments for configuration.

To configure, build and test the project with extra arguments,
you can run this sets of command.

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=20 # Your extra arguments here.
cmake --build build
ctest --test-dir build
```

> [!IMPORTANT]
>
> Beman projects are
> [passive projects](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md#cmake),
> therefore,
> you will need to specify C++ version via `CMAKE_CXX_STANDARD`
> when manually configuring the project.

### Project specific configure arguments

When configuring the project manually,
you can pass an array of project specific CMake configs to customize your build.

Project specific options are prefixed with `BEMAN_EXEMPLAR`.
You can see the list of available options with:

```bash
cmake -LH | grep "BEMAN_EXEMPLAR" -C 2
```

<details>

<summary> Details of CMake arguments. </summary>

#### `BEMAN_EXEMPLAR_BUILD_TESTS`

Enable building tests and test infrastructure. Default: ON.
Values: { ON, OFF }.

You can configure the project to have this option turned off via:

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=20 -DBEMAN_EXEMPLAR_BUILD_TESTS=OFF
```

> [!TIP]
> Because this project requires Google Tests as part of its development
> dependency,
> disable building tests avoids the project from pulling Google Tests from
> GitHub.

#### `BEMAN_EXEMPLAR_BUILD_EXAMPLES`

Enable building examples. Default: ON. Values: { ON, OFF }.

</details>

### Produce a static library

<!--
TODO
-->

Producing a static `libbeman.exemplar.a` library, ready to package with its headers:

```shell
cmake --workflow --preset gcc-debug
cmake --workflow --preset gcc-release
cmake --install build/gcc-release --prefix /opt/beman.exemplar
```

<details>
<summary> Build beman.exemplar (verbose logs) </summary>

```shell
# Configure beman.exemplar via gcc-debug workflow for development.
$ cmake --workflow --preset gcc-debug
Executing workflow step 1 of 3: configure preset "gcc-debug"

Preset CMake variables:

  CMAKE_BUILD_TYPE="Debug"
  CMAKE_CXX_COMPILER="g++"
  CMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined"
  CMAKE_CXX_STANDARD="20"

-- The CXX compiler identification is GNU 11.4.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- The C compiler identification is GNU 11.4.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Found Python3: /usr/bin/python3.10 (found version "3.10.12") found components: Interpreter
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE
-- Configuring done
-- Generating done
-- Build files have been written to: /home/runner/work/exemplar/exemplar/build/gcc-debug

Executing workflow step 2 of 3: build preset "gcc-debug"

[1/14] Building CXX object src/beman/exemplar/CMakeFiles/beman.exemplar.dir/identity.cpp.o
[2/14] Linking CXX static library src/beman/exemplar/libbeman.exemplar.a
[3/14] Building CXX object examples/CMakeFiles/beman.exemplar.examples.identity_direct_usage.dir/identity_direct_usage.cpp.o
[4/14] Linking CXX executable examples/beman.exemplar.examples.identity_direct_usage
[5/14] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[6/14] Building CXX object src/beman/exemplar/CMakeFiles/beman.exemplar.tests.dir/identity.t.cpp.o
[7/14] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[8/14] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[9/14] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[10/14] Linking CXX static library lib/libgtest.a
[11/14] Linking CXX static library lib/libgtest_main.a
[12/14] Linking CXX static library lib/libgmock.a
[13/14] Linking CXX static library lib/libgmock_main.a
[14/14] Linking CXX executable src/beman/exemplar/beman.exemplar.tests

Executing workflow step 3 of 3: test preset "gcc-debug"

Test project /home/runner/work/exemplar/exemplar/build/gcc-debug
    Start 1: IdentityTest.call_identity_with_int
1/4 Test #1: IdentityTest.call_identity_with_int ...........   Passed    0.13 sec
    Start 2: IdentityTest.call_identity_with_custom_type
2/4 Test #2: IdentityTest.call_identity_with_custom_type ...   Passed    0.01 sec
    Start 3: IdentityTest.compare_std_vs_beman
3/4 Test #3: IdentityTest.compare_std_vs_beman .............   Passed    0.01 sec
    Start 4: IdentityTest.check_is_transparent
4/4 Test #4: IdentityTest.check_is_transparent .............   Passed    0.01 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.18 sec

# Configure beman.exemplar via gcc-release workflow for direct usage.
$ cmake --workflow --preset gcc-release
Executing workflow step 1 of 3: configure preset "gcc-release"

Preset CMake variables:

  CMAKE_BUILD_TYPE="RelWithDebInfo"
  CMAKE_CXX_COMPILER="g++"
  CMAKE_CXX_FLAGS="-O3"
  CMAKE_CXX_STANDARD="20"

-- The CXX compiler identification is GNU 11.4.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- The C compiler identification is GNU 11.4.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Found Python3: /usr/bin/python3.10 (found version "3.10.12") found components: Interpreter
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE
-- Configuring done
-- Generating done
-- Build files have been written to: /home/runner/work/exemplar/exemplar/build/gcc-release

Executing workflow step 2 of 3: build preset "gcc-release"

[1/14] Building CXX object src/beman/exemplar/CMakeFiles/beman.exemplar.dir/identity.cpp.o
[2/14] Linking CXX static library src/beman/exemplar/libbeman.exemplar.a
[3/14] Building CXX object examples/CMakeFiles/beman.exemplar.examples.identity_direct_usage.dir/identity_direct_usage.cpp.o
[4/14] Linking CXX executable examples/beman.exemplar.examples.identity_direct_usage
[5/14] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[6/14] Building CXX object src/beman/exemplar/CMakeFiles/beman.exemplar.tests.dir/identity.t.cpp.o
[7/14] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[8/14] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[9/14] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[10/14] Linking CXX static library lib/libgtest.a
[11/14] Linking CXX static library lib/libgtest_main.a
[12/14] Linking CXX static library lib/libgmock.a
[13/14] Linking CXX executable src/beman/exemplar/beman.exemplar.tests
[14/14] Linking CXX static library lib/libgmock_main.a

Executing workflow step 3 of 3: test preset "gcc-release"

Test project /home/runner/work/exemplar/exemplar/build/gcc-release
    Start 1: IdentityTest.call_identity_with_int
1/4 Test #1: IdentityTest.call_identity_with_int ...........   Passed    0.00 sec
    Start 2: IdentityTest.call_identity_with_custom_type
2/4 Test #2: IdentityTest.call_identity_with_custom_type ...   Passed    0.00 sec
    Start 3: IdentityTest.compare_std_vs_beman
3/4 Test #3: IdentityTest.compare_std_vs_beman .............   Passed    0.00 sec
    Start 4: IdentityTest.check_is_transparent
4/4 Test #4: IdentityTest.check_is_transparent .............   Passed    0.00 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.01 sec

# Run examples.
$ build/gcc-release/examples/beman.exemplar.examples.identity_direct_usage
2024

```

</details>

<details>
<summary> Install beman.exemplar (verbose logs) </summary>

```shell
# Install build artifacts from `build` directory into `opt/beman.exemplar` path.
$ cmake --install build/gcc-release --prefix /opt/beman.exemplar
-- Install configuration: "RelWithDebInfo"
-- Up-to-date: /opt/beman.exemplar/lib/libbeman.exemplar.a
-- Up-to-date: /opt/beman.exemplar/include/beman/exemplar/identity.hpp


# Check tree.
$ tree /opt/beman.exemplar
/opt/beman.exemplar
├── include
│   └── beman
│       └── exemplar
│           └── identity.hpp
└── lib
    └── libbeman.exemplar.a

4 directories, 2 files
```

</details>

<details>
<summary> Disable tests build </summary>

To build this project with tests disabled (and their dependencies),
simply use `BEMAN_EXEMPLAR_BUILD_TESTING=OFF` as documented in upstream [CMake documentation](https://cmake.org/cmake/help/latest/module/CTest.html):

```shell
cmake -B build -S . -DBEMAN_EXEMPLAR_BUILD_TESTING=OFF
```

</details>

## Integrate beman.exemplar into your project

<details>
<summary> Use beman.exemplar directly from C++ </summary>
<!-- TODO Darius: rewrite section!-->

If you want to use `beman.exemplar` from your project,
you can include `beman/exemplar/*.hpp`  files from your C++ source files

```cpp
#include <beman/exemplar/identity.hpp>
```

and directly link with `libbeman.exemplar.a`

```shell
# Assume /opt/beman.exemplar staging directory.
$ c++ -o identity_usage examples/identity_usage.cpp \
    -I /opt/beman.exemplar/include/ \
    -L/opt/beman.exemplar/lib/ -lbeman.exemplar
```

</details>

<details>
<summary> Use beman.exemplar directly from CMake </summary>

<!-- TODO Darius: rewrite section! Add examples. -->

For CMake based projects, you will need to use the `beman.exemplar` CMake module to define the `beman::exemplar` CMake target:

```cmake
find_package(beman.exemplar REQUIRED)
```

You will also need to add `beman::exemplar`
to the link libraries of any libraries or executables that include `beman/exemplar/*.hpp` in their source or header file.

```cmake
target_link_libraries(yourlib PUBLIC beman::exemplar)
```

</details>

<details>
<summary> Use beman.exemplar from other build systems </summary>

<!-- TODO Darius: rewrite section! Add examples. -->

Build systems that support `pkg-config` by providing a `beman.exemplar.pc` file.
Build systems that support interoperation via `pkg-config` should be able to detect `beman.exemplar` for you automatically.

</details>

## Contributing

Please do! Issues and pull requests are appreciated.
