# sb.h

![CI](https://github.com/rsore/sb/actions/workflows/test.yml/badge.svg)

`sb.h` is a single-header library for string building in C and C++. Drop it into your project and you're good to go.
Only requirement is libc.
It’s lightweight, portable across major platforms, and public domain.

## Tested platforms and compilers

### Linux
- **Compilers:** GCC 14, Clang 20
- **C standards:** C99, C11, C17, C23
- **C++ standards:** C++11, C++14, C++17, C++20, C++23, C++26
- **Flags:** `-Wall -Wextra -Werror -pedantic-errors`

### Windows
- **Compiler:** MSVC 2022
- **C standards:** C11, C17
- **C++ standards:** C++14, C++17, C++20, C++latest
- **Flags:** `/W4 /WX`

### macOS
- **Compiler:** Apple Clang (latest)
- **C standards:** C99, C11, C17, C2x
- **C++ standards:** C++11, C++14, C++17, C++20, C++2b
- **Flags:** `-Wall -Wextra -Werror -pedantic-errors`
