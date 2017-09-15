# libsuperderpy codestyle

## clang-format

See [the configuration file](.clang-format).

## clang-tidy

```
-checks=*,-clang-analyzer-alpha.*,-hicpp-no-assembler,-google-readability-todo,
-performance-type-promotion-in-math-fn,-misc-unused-parameters,-cert-msc30-c,-cert-msc50-cpp
```

*Note:* clang-tidy runs automatically during compilation if found by CMake (can be disabled with `-DUSE_CLANG_TIDY=no`)

## Qt Creator's Code Model

```
-Weverything -Wno-missing-field-initializers -Wno-unused-parameter -Wno-padded -Wno-conversion
-Wno-double-promotion -Wno-bad-function-cast -Wno-pedantic -Wno-unused-macros -Wno-switch-enum
-std=c11 -D__codemodel__
```
