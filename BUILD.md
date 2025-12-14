# Building edfio

This document describes how to build and test the edfio library.

## Requirements

- CMake 3.21 or higher
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- Ninja (optional, but recommended)

## Quick Start

### Using CMake Presets (Recommended)

```bash
# Configure for debug build with tests
cmake --preset debug

# Build
cmake --build build/debug

# Run tests
ctest --test-dir build/debug
```

### Available Presets

- **debug**: Debug build with tests and examples
- **release**: Optimized release build
- **sanitize**: Debug build with address and undefined behavior sanitizers
- **lint**: Debug build with clang-tidy enabled
- **ci**: CI build configuration with sanitizers

### Manual Configuration

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DEDFIO_BUILD_TESTS=ON

# Build
cmake --build .

# Test
ctest --output-on-failure
```

## Build Options

- `EDFIO_BUILD_TESTS` (default: ON if main project): Build unit and integration tests
- `EDFIO_BUILD_EXAMPLES` (default: ON if main project): Build example programs
- `EDFIO_ENABLE_SANITIZERS` (default: OFF): Enable sanitizers (address, undefined behavior)
- `EDFIO_ENABLE_CLANG_TIDY` (default: OFF): Enable clang-tidy static analysis
- `EDFIO_ENABLE_CPPCHECK` (default: OFF): Enable cppcheck static analysis

## Code Quality Tools

### Formatting with clang-format

```bash
# Format all source files
find tests examples -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
```

### Linting with clang-tidy

```bash
# Build with clang-tidy enabled
cmake --preset lint
cmake --build build/lint
```

### Running with Sanitizers

```bash
# Build with sanitizers
cmake --preset sanitize
cmake --build build/sanitize

# Run tests with sanitizers
ctest --test-dir build/sanitize
```

## Using as a Dependency

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    edfio
    GIT_REPOSITORY https://github.com/idotta/edfio.git
    GIT_TAG main
)
FetchContent_MakeAvailable(edfio)

target_link_libraries(your_target PRIVATE edfio::edfio)
```

### CMake find_package

After installing edfio:

```cmake
find_package(edfio REQUIRED)
target_link_libraries(your_target PRIVATE edfio::edfio)
```

## Installation

```bash
# Configure with installation prefix
cmake --preset release -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
cmake --build build/release

# Install (may require sudo)
cmake --install build/release
```

## Testing

### Run All Tests

```bash
ctest --test-dir build/debug
```

### Run Specific Test Suite

```bash
# Unit tests only
ctest --test-dir build/debug -L unit

# Integration tests only
ctest --test-dir build/debug -L integration
```

### Run Specific Test

```bash
ctest --test-dir build/debug -R DataFormatTest
```

## Examples

After building, run the examples:

```bash
./build/debug/examples/basic_usage
```

## Continuous Integration

The project includes a `ci` preset optimized for CI environments:

```bash
cmake --preset ci
cmake --build build/ci
ctest --test-dir build/ci
```

## Troubleshooting

### Ninja not found

If Ninja is not available, CMake will fall back to the default generator (Make on Unix, Visual Studio on Windows).

### Compiler not found

Specify the compiler explicitly:

```bash
cmake --preset debug -DCMAKE_CXX_COMPILER=g++-13
```

### Sanitizer errors

Some sanitizers are incompatible with each other. The default sanitizer configuration enables address and undefined behavior sanitizers, which are compatible. To use thread or memory sanitizers, configure them individually in `cmake/Sanitizers.cmake`.
