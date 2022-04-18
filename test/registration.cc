#include <naah.h>
#include <naah_friend.h>

#include <iostream>

namespace {
uint32_t Add(uint32_t a, uint32_t b) { return a + b; }

class Calculator : public naah::Class {
  uint32_t _num;
  Calculator(uint32_t num) : _num(num) {}

  uint32_t add(uint32_t a) {
    _num += a;
    return _num;
  }

  uint32_t num() { return _num; }

  void set_num(uint32_t num) { _num = num; }

  static uint32_t _count;

  static uint32_t addCount(uint32_t a) {
    _count += a;
    return _count;
  }

  static uint32_t count() { return _count; }

  static void set_count(uint32_t count) { _count = count; }

  static Calculator create(uint32_t num) { return Calculator(num); }

  NAAH_FRIEND
};

uint32_t Calculator::_count = 0;

struct MyObject : naah::Object {
  std::string str;
  std::optional<uint32_t> num;
};

MyObject MyObjectMethod(MyObject input) {
  return MyObject{{},
                  input.str + " world",
                  input.num.has_value()
                      ? std::optional<uint32_t>(*input.num + 1)
                      : std::nullopt};
}

}  // namespace

NAAH_REGISTRATION {
  using reg = naah::Registration;

  reg::Value("num", 233);
  reg::Value("str", "hello world");
  reg::Function("add",
                [](uint32_t a, uint32_t b) -> uint32_t { return a + b; });
  reg::Function<Add>("addTpl");

  reg::Object<MyObject>().Member<&MyObject::num>("num").Member<&MyObject::str>(
      "str");
  reg::Function<MyObjectMethod>("myObjectMethod");

  reg::Class<Calculator, uint32_t>("Calculator")
      .InstanceMethod<&Calculator::add>("add")
      .InstanceAccessor<&Calculator::num, &Calculator::set_num>("num")
      .InstanceAccessor<&Calculator::num>("readonlyNum")
      .StaticMethod<&Calculator::addCount>("add")
      .StaticMethod<&Calculator::create>("create")
      .StaticAccessor<&Calculator::count, &Calculator::set_count>("count")
      .StaticAccessor<&Calculator::count>("readonlyCount");
}

NAAH_EXPORT
