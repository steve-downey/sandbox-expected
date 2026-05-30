TODO

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++20 standard or greater
* CMake 3.30 or later
* (Test Only) GoogleTest

You can disable building tests by setting CMake option `BEMAN_EXPECTED_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 16-13   | C++26-C++17   | libstdc++         |
| GCC        | 12-11   | C++23-C++17   | libstdc++         |
| Clang      | 22-19   | C++26-C++17   | libstdc++, libc++ |
| Clang      | 18      | C++26-C++17   | libc++            |
| Clang      | 18      | C++23-C++17   | libstdc++         |
| Clang      | 17      | C++26-C++17   | libc++            |
| Clang      | 17      | C++20, C++17  | libstdc++         |
| AppleClang | latest  | C++26-C++17   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.expected into your project

### Build

You can build expected using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.expected without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

#### Vcpkg

The preferred way to install expected is via vcpkg. To do so, after installing vcpkg
itself, you need to add support for the Beman project's [vcpkg
registry](https://github.com/bemanproject/vcpkg-registry) by configuring a
`vcpkg-configuration.json` file (which expected [provides](vcpkg-configuration.json)).

Then, simply run `vcpkg install beman-expected`.

#### Manual

To install beman.expected globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── expected
│           ├── expected.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.expected
            ├── beman.expected-config-version.cmake
            ├── beman.expected-config.cmake
            └── beman.expected-targets.cmake
```

### CMake Configuration

If you installed beman.expected to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.expected` package to define the `beman::expected` CMake
target:

```cmake
find_package(beman.expected REQUIRED)
```

You will then need to add `beman::expected` to the link libraries of any libraries or
executables that include `beman.expected` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::expected)
```

### Using beman.expected

To use `beman.expected` in your C++ project,
include an appropriate `beman.expected` header from your source code.

```c++
#include <beman/expected/expected.hpp>
```

> [!NOTE]
>
> `beman.expected` headers are to be included with the `beman/expected/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <expected.hpp>`) is unsupported.
