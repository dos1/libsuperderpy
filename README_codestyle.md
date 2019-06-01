# libsuperderpy codestyle

## clang-format

See [the configuration file](.clang-format).

## clang-tidy

See [the configuration file](.clang-tidy).

*Note:* clang-tidy can run automatically during compilation by enabling it with `-DUSE_CLANG_TIDY=ON`

## Qt Creator's Code Model

```
-Weverything -Wno-missing-field-initializers -Wno-unused-parameter -Wno-padded -Wno-conversion
-Wno-double-promotion -Wno-bad-function-cast -Wno-pedantic -Wno-unused-macros -Wno-switch-enum
-Wno-disabled-macro-expansion -D__codemodel__
```
