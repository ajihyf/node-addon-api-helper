# Error Handling

## With C++ Exception

When C++ exception is enabled, throw `naah::{Error, RangeError, TypeError}` directly in your C++ code.

```cpp
uint32_t TestNum(uint32_t num) {
  if (num < 10) {
    throw naah::RangeError("num too small");
  }
  return num - 10;
}
```

## Without C++ Exception

When C++ exception is disabled, use `naah::Result<T, E>` as return value, where `E` is one of `naah::Error`, `naah::TypeError` or `naah::RangeError`.

```cpp
naah::Result<std::string, naah::RangeError> FunctionThrowsWithResult(
    uint32_t i) {
  if (i > 233) {
    return naah::RangeError("bigger than 233");
  }
  return std::to_string(i);
}
```
