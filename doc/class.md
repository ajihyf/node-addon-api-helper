# Class

To export a class `T`, inherit `naah::Class` and use `naah::Registration::Class` to register class with methods.

## Constructor

Since it's impossible to get the address of a C++ constructor, you need to specify constructor arguments explicitly :

```cpp
class A : public naah::Class {
 public:
  A(uint32_t, std::string);
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<A>("A")
      .Constructor<uint32_t, std::string>();
}
```

If no constructor is specified, calling new on this class will throw a JavaScript Error.
But you can still create the object directly in C++(like factory patterns), see [Class as Argument](#class-as-argument) for details.

## InstanceMethod

```cpp
class A : public naah::Class {
 public:
  void hello();
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<A>("A")
      .InstanceMethod<&A::hello>("hello");
}
```

## InstanceAccessor

```cpp
class A : public naah::Class {
 public:
  uint32_t num();
  void set_num(uint32_t);
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<A>("A")
      .InstanceAccessor<&A::num, &A::set_num>("num");
}
```

## StaticMethod

```cpp
class A : public naah::Class {
 public:
  static void hello();
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<A>("A")
      .StaticMethod<&A::hello>("hello");
}
```

## StaticAccessor

```cpp
class A : public naah::Class {
 public:
  static uint32_t num();
  static void set_num(uint32_t);
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<A>("A")
      .StaticAccessor<&A::num, &A::set_num>("num");
}
```

## Class Friendship

To export private methods :

```cpp
#include <naah_friend.h>

class A : public naah::Class {
  void privateMethod();

  NAAH_FRIEND
};
```

## Class as Argument

Class instances can also be function arguments and return values.

```cpp
class Foo : public naah::Class {
 public:
  Foo(uint32_t num);
  uint32_t num;
};

class Bar : public naah::Class {
 public:
  Foo NewFoo(Foo* foo, uint32_t num) {
    return Foo(foo->num + num);
  }
};
```

Note that the argument type is a non-null pointer. Since the instance is managed by JavaScript VM, you should never delete it or pass it to other threads.

## Inheritance

Use `Inherit<Base>()` to allow calling parent class methods (including instance and static methods) :

```cpp
class Base : public naah::Class {
 public:
  Base(uint32_t num) : _num(num) {}

  virtual std::string GetReal() = 0;

  uint32_t num() { return _num; }
  void set_num(uint32_t num) { _num = num; }

  uint32_t Add(uint32_t num) {
    _num += num;
    return _num;
  }

  uint32_t _num;

  static std::string GetRealStatic(Base* base) { return base->GetReal(); }
};

class SubA : public Base {
 public:
  SubA(uint32_t num) : Base(num) {}

  std::string GetReal() override { return "A"; }

  uint32_t Sub(uint32_t num) {
    _num -= num;
    return _num;
  }
};

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Class<Base>("Base")
      .InstanceMethod<&Base::GetReal>("getReal")
      .InstanceMethod<&Base::Add>("add")
      .InstanceAccessor<&Base::num, &Base::set_num>("num")
      .StaticMethod<Base::GetRealStatic>("getReal");
  reg::Class<SubA>("SubA")
      .Inherits<Base>()
      .Constructor<uint32_t>()
      .InstanceMethod<&SubA::Sub>("sub");
}
```

In JavaScript :

```js
const a = new binding.SubA(42);
console.log(a.num); // 42
console.log(binding.Base.getReal(a)); // 'A'
console.log(binding.SubA.getReal(a)); // 'A'
console.log(a.add(1)); // 43
console.log(a.sub(1)); // 42
```

However, the support of inheritance has some limitations:

- Due to [node-addon-api#229](https://github.com/nodejs/node-addon-api/issues/229), **naah** internally copies parent class methods to child class and monkey patches the prototype chain. It should work in most cases but doesn't match 100% with pure ES class inheritance.
- Only single inheritance in JavaScript(calls to `Inherit<Base>()`) is supported.
