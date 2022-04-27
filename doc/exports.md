# Exports

Declare your exports inside macro `NAAH_REGISTRATION` and use `NAAH_EXPORT` to register your native module.

```cpp
NAAH_REGISTRATION {
  // ... your exports code
}

NAAH_EXPORT
```

## Constant Value

```cpp
NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Value("num", 233);
  reg::Value("str", "hello world");
}
```

## Function

```cpp
uint32_t Add(uint32_t a, uint32_t b) { return a + b; }

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Function("add",
                [](uint32_t a, uint32_t b) -> uint32_t { return a + b; });
  reg::Function<Add>("addTpl");
}
```

For more information about function exports, see [Function](./function.md).

## Class

```cpp
class Calculator : public naah::Class {
 public:
  uint32_t _num;
  Calculator(uint32_t num): _num(num) {}
  uint32_t add(uint32_t num) {
    return _num + num;
  }
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<Calculator>("Calculator")
      .Constructor<uint32_t>()
      .InstanceMethod<&Calculator::add>("add")
}
```

For more information about class exports, see [Class](./class.md).
