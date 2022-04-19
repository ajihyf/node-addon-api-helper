# Function

Write function in pure C++ and export it as JavaScript function with `naah::Registration::Function`.

**naah** will take care of value transformation between C++ and JavaScript (see [ValueTransformer](./value_transformer.md) for details).

If any type mismatch occurs, a `TypeError` will be thrown automatically.

```cpp
#include <naah.h>

namespace {

std::string helloWorld(std::string str) { return str + " world"; }

}  // namespace

NAAH_REGISTRATION {
  using reg = naah::Registration;
  // lambda is supported
  reg::Function("add",
                [](uint32_t a, uint32_t b) -> uint32_t { return a + b; });
  reg::Function<helloWorld>("helloWorld");
}

NAAH_EXPORT
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
| naah::Undefined                                   | undefined                                 |
| naah::Null                                        | null                                      |
| bool                                              | boolean                                   |
| [u]int[8,16,32]\_t                                | number                                    |
| [u]int64_t                                        | BigInt                                    |
| float, double                                     | number                                    |
| std::function<void(Args...)>                      | (args...) => void                         |
| std::string, std::u16string                       | string                                    |
| std::vector\<T>                                   | T[]                                       |
| std::variant\<T1, T2, ...>                        | T1 \| T2 \| ...                           |
| std::optional\<T>                                 | T \| undefined                            |
| std::tuple\<T1, T2, ...>                          | [T1, T2, ...]                             |
| naah::{Int8Array, Uint8Array, etc.}               | Int8Array, Uint8Array                     |
| T (inherits [naah::Object](./object.md))          | object of interface T                     |
| T\* (inherits [naah::Class](./class.md))          | instance of class T                       |
| Napi::{Object, Array, Function, TypedArray, etc.} | Object, Array, Function, TypedArray, etc. |

`std::function<void(Args...)>` arguments are [Thread Safe Functions](./thread_safe_function.md), they are safe to call in any thread.

### Difference between C++ values and JavaScript values

`T*` and `Napi::...` are **JavaScript** values. Which means their lifetimes are managed by JavaScript VM. You should never pass or access them out of JavaScript call stack.

All other types are pure C++. Their contents are copied from JavaScript values, which means they are safe to use out of JavaScript stack.

In most cases, you should use C++ values. Except for the scenario you want to access JavaScript contents in place, for example, set a key to an input object, change input TypedArray data, access input TypedArray data without copy, etc.

## Return Type

Supported return types :

| C++                                                 | JavaScript                                |
| --------------------------------------------------- | ----------------------------------------- |
| naah::Undefined, void                               | undefined                                 |
| naah::Null                                          | null                                      |
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
| naah::{Int8Array, Uint8Array, etc.}                 | Int8Array, Uint8Array                     |
| naah::{Error, RangeError, TypeError}                | Error, RangeError, TypeError              |
| T (inherits [naah::Object](./object.md))            | object of interface T                     |
| T (inherits [naah::Class](./class.md))              | instance of class T                       |
| Napi::{Object, Array, Function, TypedArray, etc.}   | Object, Array, Function, TypedArray, etc. |

`naah::{Error, RangeError, TypeError}` will be transformed to JavaScript error values as return value. To throw exceptions, see [Error Handling](./error_handling.md).

## Inject CallbackInfo

In some cases, you may want to access JavaScript land in native functions.

For this scenario, declare the first argument type as `const Napi::CallbackInfo&`, **naah** will automatically inject it.

```cpp
uint32_t addArgsLength(const Napi::CallbackInfo& info, uint32_t num) {
  return num + info.Length();
}
```
