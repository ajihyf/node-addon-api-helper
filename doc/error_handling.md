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

When C++ exception is disabled, use `Napi::{Error, RangeError, TypeError}` combined with `std::optional`.

```cpp
std::optional<uint32_t> TestNum(const Napi::CallbackInfo &info, uint32_t num) {
  if (num < 10) {
    Napi::RangeError::New(info.Env(), "num too small").ThrowAsJavaScriptException();
    return {};
  }
  return num - 10;
}
```

### use std::expected

If you are using `std::expected` or some other result handling types.
You can try this approach.

First, add a template specification :

```cpp
namespace naah {
template <typename T, typename E>
struct ValueTransformer<std::expected<T, E>> {
  static Napi::Value ToJS(Napi::Env env, const std::expected<T, E> &v) {
    if (v) {
      return ValueTransformer<T>::ToJS(env, *v);
    }
    E::JSError::New(env, v.error().Message()).ThrowAsJavaScriptException(env);
    return env.Undefined();
  }
};
}
```

Then you can return a `std::expected` value.

```cpp
std::expected<uint32_t, naah::RangeError> TestNum(uint32_t num) {
  if (num < 10) {
    return naah::RangeError("num too small");
  }
  return num - 10;
}
```
