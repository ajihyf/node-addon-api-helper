# Function

Write function in pure C++ and export it as JavaScript function with `NapiHelper::Registration::Function`.

**node-addon-api-helper** will take care of value transformation between C++ and JavaScript (see [ValueTransformer](./value_transformer.md) for details).

If any type mismatch occurs, a `TypeError` will be thrown automatically.

```cpp
#include <napi_helper.h>

namespace {

std::string helloWorld(std::string str) { return str + " world"; }

}  // namespace

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;
  // lambda is supported
  reg::Function("add",
                [](uint32_t a, uint32_t b) -> uint32_t { return a + b; });
  reg::Function<helloWorld>("helloWorld");
}

NAPI_HELPER_EXPORT
```

In JavaScript land :

```javascript
console.log(binding.add(42, 3)); // 45
console.log(binding.helloWorld("hello")); // 'hello world'
console.log(binding.helloWorld(42)); // throws TypeError
```

## Arguments

Supported argument types :

| C++                                               | JavaScript                                |
| ------------------------------------------------- | ----------------------------------------- |
| NapiHelper::Undefined                             | undefined                                 |
| NapiHelper::Null                                  | null                                      |
| bool                                              | boolean                                   |
| [u]int[8,16,32]\_t                                | number                                    |
| [u]int64_t                                        | BigInt                                    |
| float, double                                     | number                                    |
| std::string, std::u16string                       | string                                    |
| std::vector\<T>                                   | T[]                                       |
| std::variant\<T1, T2, ...>                        | T1 \| T2 \| ...                           |
| std::optional\<T>                                 | T \| undefined                            |
| std::tuple\<T1, T2, ...>                          | [T1, T2, ...]                             |
| Napi::{Object, Array, Function, TypedArray, etc.} | Object, Array, Function, TypedArray, etc. |
| T\*                                               | instance of [Class](./class.md) T         |

## Return Type

Supported return types :

| C++                                                 | JavaScript                                |
| --------------------------------------------------- | ----------------------------------------- |
| NapiHelper::Undefined, void                         | undefined                                 |
| NapiHelper::Null                                    | null                                      |
| bool                                                | boolean                                   |
| [u]int[8,16,32]\_t                                  | number                                    |
| [u]int64_t                                          | BigInt                                    |
| float, double                                       | number                                    |
| std::string, std::u16string                         | string                                    |
| lambda or std::function of <R(Args...)>             | (args: Args...) => R                      |
| const char\*, std::string_view, std::u16string_view | string                                    |
| std::vector\<T>                                     | T[]                                       |
| std::variant\<T1, T2, ...>                          | T1 \| T2 \| ...                           |
| std::optional\<T>                                   | T \| undefined                            |
| std::tuple\<T1, T2, ...>                            | [T1, T2, ...]                             |
| Napi::{Object, Array, Function, TypedArray, etc.}   | Object, Array, Function, TypedArray, etc. |
| NapiHelper::{Error, RangeError, TypeError}          | Error, RangeError, TypeError              |

## Inject CallbackInfo

In some cases, you may want to access JavaScript land in native functions.

For this scenario, declare the first argument type as `const Napi::CallbackInfo&`, **node-addon-api-helper** will automatically inject it.

```cpp
uint32_t addArgsLength(const Napi::CallbackInfo& info, uint32_t num) {
  return num + info.Length();
}
```

## Custom Type

To support a custom type as function arguments and return value, just add a template specification of `ValueTransformer<T>` :

```cpp
namespace {
struct CustomStruct {
  std::string str;
  std::optional<uint32_t> num;
};

CustomStruct CustomMethod(CustomStruct input) {
  return CustomStruct{
      input.str + " world",
      input.num.has_value() ? std::optional<uint32_t>(*input.num + 1)
                            : std::optional<uint32_t>({}),
  };
}
}  // namespace


namespace NapiHelper {
template <>
struct ValueTransformer<CustomStruct> {
  static std::optional<CustomStruct> FromJS(Napi::Value v) {
    if (!v.IsObject()) {
      return {};
    }
    Napi::Object obj = v.As<Napi::Object>();
    auto str = ValueTransformer<std::string>::FromJS(obj.Get("str"));
    if (!str) {
      return {};
    }
    auto num = ValueTransformer<uint32_t>::FromJS(obj.Get("num"));
    return CustomStruct{std::move(*str), num};
  }
  static Napi::Value ToJS(Napi::Env env, const CustomStruct &v) {
    Napi::Object obj = Napi::Object::New(env);
    if (v.num) {
      obj.Set("num", ValueTransformer<uint32_t>::ToJS(env, *v.num));
    }
    obj.Set("str", ValueTransformer<std::string>::ToJS(env, v.str));
    return obj;
  }
};
}  // namespace NapiHelper
```
