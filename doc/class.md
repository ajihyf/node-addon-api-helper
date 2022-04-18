# Class

To export a class `T`, inherit `NapiHelper::Class` and use `NapiHelper::Registration::Class` to register class with methods.

## Constructor

Since it's impossible to get the address of a C++ constructor, you need to specify constructor arguments explicitly :

```cpp
class A: public NapiHelper::Class {
 public:
  A(uint32_t, std::string);
};

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Class<A, uint32_t, std::string>("A");
}
```

## InstanceMethod

```cpp
class A {
 public:
  void hello();
};

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Class<A>("A")
      .InstanceMethod<&A::hello>("hello");
}
```

## InstanceAccessor

```cpp
class A {
 public:
  uint32_t num();
  void set_num(uint32_t);
};

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Class<A>("A")
      .InstanceAccessor<&A::num, &A::set_num>("num");
}
```

## StaticMethod

```cpp
class A {
 public:
  static void hello();
};

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Class<A>("A")
      .StaticMethod<&A::hello>("hello");
}
```

## StaticAccessor

```cpp
class A {
 public:
  static uint32_t num();
  static void set_num(uint32_t);
};

NAPI_HELPER_REGISTRATION {
  using reg = NapiHelper::Registration;

  reg::Class<A>("A")
      .StaticAccessor<&A::num, &A::set_num>("num");
}
```

## Class Friendship

To export private methods :

```cpp
#include <napi_helper_friend.h>

class A {
  void privateMethod();

  NAPI_HELPER_FRIEND
};
```

## Class as Argument

Exported class object can also be function arguments and return value. This feature is supported for NAPI_VERSION >= 8.

```cpp
class Foo : public NapiHelper::Class {
 public:
  Foo(uint32_t num);
  uint32_t num;
};

class Bar : public NapiHelper::Class {
 public:
  Foo NewFoo(Foo* foo, uint32_t num) {
    return Foo(foo->num + num);
  }
};
```

Note that the argument type is a non-null pointer. Since the instance is managed by JavaScript VM, you should never delete it or pass it to other threads.
