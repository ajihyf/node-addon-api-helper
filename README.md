# node-addon-api-helper

[![NPM](https://img.shields.io/npm/v/node-addon-api-helper)](https://www.npmjs.com/package/node-addon-api-helper)
[![CI](https://github.com/ajihyf/node-addon-api-helper/workflows/ci/badge.svg)](https://github.com/ajihyf/node-addon-api-helper/actions/workflows/ci.yml)
[![lint](https://github.com/ajihyf/node-addon-api-helper/workflows/lint/badge.svg)](https://github.com/ajihyf/node-addon-api-helper/actions/workflows/lint.yml)
[![LICENSE](https://img.shields.io/github/license/ajihyf/node-addon-api-helper)](https://github.com/ajihyf/node-addon-api-helper/blob/main/LICENSE)

**node-addon-api-helper** provides a more convenient way to write Node.js C++ addons.

# Table of contents

- [Installation](#installation)
- [Usage](#usage)
  - [ValueTransformer](#valuetransformer)
    - [Custom Type](#customtype)
  - [Function](#function)
    - [Optional Arguments](#optional-arguments)
    - [Union Types](#union-types)
    - [Lambda](#lambda)
    - [Inject CallbackInfo](#inject-callbackinfo)
  - [ScriptWrappable](#scriptwrappable)
    - [ScriptWrappable as Argument](#scriptwrappable-as-argument)
  - [Error Handling](#error-handling)
    - [With C++ Exception](#with-c-exception)
    - [Without C++ Exception](#without-c-exception)
- [Thanks](#thanks)

# Installation

```bash
npm install node-addon-api node-addon-api-helper
```

Add following content to your GYP file.

```gyp
{
  'include_dirs': [
    "<!(node -p \"require('node-addon-api').include_dir\")",
    "<!(node -p \"require('node-addon-api-helper').include_dir\")",
  ]
}
```

**node-addon-api-helper** heavily depends on **C++17**, if you haven't configured your project on C++17, have a look at [common.gypi](./common.gypi) as an example.

# Usage

## Function

Write function in pure C++ and export it as JavaScript function with `NapiHelper::Function::New`.

**node-addon-api-helper** will take care of value transformation between C++ and JavaScript (see [ValueTransformer](#valuetransformer) for details).

If any type mismatch occurs, a `TypeError` will be thrown automatically.

Example :

```cpp
#include <napi_helper.h>

namespace {

std::string Method(std::string str) { return str + " world"; }

void VoidMethod(Napi::Object obj) {
  obj.Set("key", Napi::String::New(obj.Env(), "value"));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("HelloWorld", NapiHelper::Function::New<Method>(env));
  exports.Set("ObjectMethod", NapiHelper::Function::New<VoidMethod>(env));
  return exports;
}

NODE_API_MODULE(binding, Init)
}  // namespace
```

In JavaScript land :

```javascript
console.log(binding.HelloWorld("hello")); // 'hello world'
console.log(binding.HelloWorld(42)); // throws TypeError
const obj = {};
binding.ObjectMethod(obj);
console.log(obj); // { key: 'value' }
binding.ObjectMethod("hello"); // throws TypeError
```

### Optional Arguments

To define an optional argument, use `std::optional` :

```cpp
uint32_t plusOne(std::optional<uint32_t> opt_num) {
  return opt_num.value_or(42) + 1;
}
```

### Union Types

To define a union type, use `std::variant` :

```cpp
uint32_t plusOneOrStrLen(std::variant<uint32_t, std::string> num_or_str) {
  if (auto *num = std::get_if<uint32_t>(&num_or_str)) {
    return *num + 1;
  }
  if (auto *str = std::get_if<std::string>(&num_or_str)) {
    return str->size();
  }
  return 0;
}
```

### Lambda

Use the non-template overload to create function with lambda :

```cpp
exports["lambdaWithCapture"] = NapiHelper::Function::New(
    env, [num = std::make_unique<uint32_t>(233)](int32_t num1) -> uint32_t {
      return num1 + *num;
    });
```

### Inject CallbackInfo

In some cases, you may want to access JavaScript land in native functions.

For this scenario, declare the first argument type as `const Napi::CallbackInfo&`, **node-addon-api-helper** will automatically inject it.

```cpp
uint32_t addArgsLength(const Napi::CallbackInfo& info, uint32_t num) {
  return num + info.Length();
}
```

## ScriptWrappable

`ScriptWrappable` is a replacement of `Napi::ObjectWrap` which provides a way to bind native class to JavaScript object.

```cpp
class TestObject {
 public:
  using Wrapped = NapiHelper::ScriptWrappable<TestObject>;

  TestObject(uint32_t num) : _num(num) {}

  void Add(uint32_t add) { _num += add; }
  void set_num(uint32_t num) { _num = num; }
  uint32_t num() { return _num; }

  static void AddStatic(uint32_t c) { _count += c; }
  static uint32_t count() { return _count; }
  static void set_count(uint32_t n) { _count = n; }

  static Napi::Function DefineClass(Napi::Env env) {
    return Wrapped::DefineClass<uint32_t>(
        env, "TestObject",
        {Wrapped::StaticMethod<TestObject::AddStatic>("add"),
         Wrapped::StaticAccessor<TestObject::count, TestObject::set_count>(
             "count"),

         Wrapped::InstanceMethod<&TestObject::Add>("add"),
         Wrapped::InstanceAccessor<&TestObject::num, &TestObject::set_num>(
             "num")});
  }

 private:
  static uint32_t _count;
  uint32_t _num;
};

uint32_t TestObject::_count = 0;
```

Since it's impossible to get the address of constructor function in C++, you should specify constructor signature explicitly like `DefineClass<CtorArg0, CtorArg1, ...>`.

### ScriptWrappable as Argument

ScriptWrappable object can also be function arguments or return value. This feature is supported for NAPI_VERSION >= 8.

```cpp
class AnotherTestObject {
 public:
  using Wrapped = NapiHelper::ScriptWrappable<AnotherTestObject>;

  TestObject::Wrapped* AddTest(TestObject::Wrapped* obj, uint32_t num) {
    obj->wrapped().Add(num);
    return obj;
  }

  static Napi::Function DefineClass(Napi::Env env) {
    return Wrapped::DefineClass(env, "AnotherTestObject",
        {Wrapped::InstanceMethod<&AnotherTestObject::AddTest>("addTest")});
  }
};
```

## ValueTransformer

`ValueTransformer` is the template class to convert native types from and into JavaScript.

```cpp
std::optional<uint32_t> opt_num = NapiHelper::ValueTransformer<uint32_t>::FromJS(value);
Napi::Value str = NapiHelper::ValueTransformer<std::string>::ToJS(env, "hello");

// There is also a helper struct `Convert`
Napi::Value num = NapiHelper::Convert::ToJS(env, 42);
```

- All `T` which has a template specification of `ValueTransformer<T>::FromJS` could be treated as function arguments.

- All `T` which has a template specification of `ValueTransformer<T>::ToJS` could be treated as function return value.

Supported types

| C++                                                 | JavaScript                                |
| --------------------------------------------------- | ----------------------------------------- |
| NapiHelper::Undefined                               | undefined                                 |
| NapiHelper::Null                                    | null                                      |
| bool                                                | boolean                                   |
| [u]int[8,16,32]\_t                                  | number                                    |
| [u]int64_t                                          | BigInt                                    |
| float, double                                       | number                                    |
| std::string, std::u16string                         | string                                    |
| const char\*, std::string_view, std::u16string_view | string                                    |
| std::vector\<T>                                     | T[]                                       |
| std::variant\<T1, T2, ...>                          | T1 \| T2 \| ...                           |
| std::optional\<T>                                   | T \| undefined                            |
| std::tuple\<T1, T2, ...>                            | [T1, T2, ...]                             |
| Napi::{Object, Array, Function, TypedArray, etc.}   | Object, Array, Function, TypedArray, etc. |
| NapiHelper::ScriptWrappable\<T>\*                   | exported T binding class                  |

Note that `const char*`, `std::string_view` and `std::u16string_view` can only be function return type.

### Custom Type

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

## Error Handling

### With C++ Exception

When C++ exception is enabled, throw `NapiHelper::{Error, RangeError, TypeError}` directly in your C++ code.

```cpp
uint32_t TestNum(uint32_t num) {
  if (num < 10) {
    throw NapiHelper::RangeError("num too small");
  }
  return num - 10;
}
```

### Without C++ Exception

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

## Thanks

- [node-addon-api](https://github.com/nodejs/node-addon-api)
- [napi-rs](https://napi.rs)
