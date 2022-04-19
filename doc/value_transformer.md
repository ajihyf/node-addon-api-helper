# ValueTransformer

`ValueTransformer` is the template class to convert native types from and into JavaScript.

```cpp
template <typename T, typename Enabled = void>
struct ValueTransformer {
  static std::optional<T> FromJS(Napi::Value);

  static Napi::Value ToJS(Napi::Env, const T &);
};

// There is also a helper function `ConvertToJS`
template <typename T>
Napi::Value ConvertToJS(Napi::Env env, T v);
```

- All `T` which has a template specification of `ValueTransformer<T>::FromJS` could be treated as function arguments.

- All `T` which has a template specification of `ValueTransformer<T>::ToJS` could be treated as function return value.
