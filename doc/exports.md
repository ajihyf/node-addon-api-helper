# Exports

Declare your exports inside macro `NAPI_HELPER_REGISTRATION` and use `NAPI_HELPER_EXPORT` to register your native module.

```cpp
NAPI_HELPER_REGISTRATION {
  // ... your exports code
}

NAPI_HELPER_EXPORT
```

## Constant Value

```cpp
NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Value("num", 233);
  reg::Value("str", "hello world");
}
```

## Function

```cpp
uint32_t Add(uint32_t a, uint32_t b) { return a + b; }

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Function("add",
                [](uint32_t a, uint32_t b) -> uint32_t { return a + b; });
  reg::Function<Add>("addTpl");
}
```

For more information about function exports, see [Function](./function.md).

## Class

```cpp
class Calculator {
 public:
  uint32_t _num;
  Calculator(uint32_t num): _num(num) {}
  uint32_t add(uint32_t num) {
    return _num + num;
  }
};

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Class<Calculator, uint32_t>("Calculator")
      .InstanceMethod<&Calculator::add>("add")
}
```

For more information about class exports, see [Class](./class.md).
