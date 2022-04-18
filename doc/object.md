# Object

To support a custom object `T` as function arguments and return value, register it with `NapiHelper::Registration::Object` and use `NapiHelper::Object<T>` as argument or return type. Note that `T` should be [default constructible](https://en.cppreference.com/w/cpp/types/is_default_constructible) and contains only public members.

```cpp
struct MyObject {
  std::string str;
  std::optional<uint32_t> num;
};

NapiHelper::Object<MyObject> MyObjectMethod(
    NapiHelper::Object<MyObject> input) {
  return MyObject{
      input.str + " world",
      input.num.has_value() ? std::optional<uint32_t>(*input.num + 1)
                            : std::nullopt,
  };
}

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Object<MyObject>()
      .Member<&MyObject::num>("num")
      .Member<&MyObject::str>("str");
  reg::Function<MyObjectMethod>("myObjectMethod");
}
```

In JavaScript :

```javascript
binding.myObjectMethod({ str: "hello" }); // { str: 'hello world' }
binding.myObjectMethod({ str: "hello", num: 42 }); // { str: 'hello world', num: 43 }
binding.myObjectMethod({}); // throws TypeError
```
