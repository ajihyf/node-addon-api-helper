# Object

To support a custom object `T` as function arguments and return value, inherit `naah::Object` and register it with `naah::Registration::Object`. Note that `T` should be [default constructible](https://en.cppreference.com/w/cpp/types/is_default_constructible) and contains only public members.

```cpp
struct MyObject : naah::Object {
  std::string str;
  std::optional<uint32_t> num;
};

MyObject MyObjectMethod(MyObject input) {
  return MyObject{
      {},
      input.str + " world",
      input.num.has_value() ? std::optional<uint32_t>(*input.num + 1)
                            : std::nullopt,
  };
}

NAAH_REGISTRATION {
  using reg = naah::Registration;

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
