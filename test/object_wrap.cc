#include <naah.h>

namespace {
class TestObject : public naah::Class {
 public:
  TestObject(uint32_t num) : _num(num) {}

  uint32_t Multiply(uint32_t num) { return _num * num; }

  void Add(uint32_t add) { _num += add; }

  void set_num(uint32_t num) { _num = num; }

  uint32_t num() const { return _num; }

  uint32_t GetArgLength(const Napi::CallbackInfo& info) {
    return info.Length();
  }

  std::string SymMethod(std::string str) { return str + "??sym"; }

  static void AddStatic(uint32_t c) { _count += c; }
  static uint32_t count() { return _count; }
  static void set_count(uint32_t n) { _count = n; };

 private:
  static uint32_t _count;
  uint32_t _num;
};

uint32_t TestObject::_count = 0;

class AnotherTestObject : public naah::Class {
 public:
  AnotherTestObject(const Napi::CallbackInfo&) {}

  void AddTest(TestObject* obj, uint32_t num) { obj->Add(num); }

  uint32_t GetTestNum(const TestObject* obj) { return obj->num(); }
};
}  // namespace

NAAH_REGISTRATION {
  naah::Registration::Class<TestObject>("TestObject")
      .Constructor<uint32_t>()
      .StaticMethod<TestObject::AddStatic>("add")
      .StaticAccessor<TestObject::count>("readonlyCount")
      .StaticAccessor<TestObject::count, TestObject::set_count>("count")
      .InstanceMethod<&TestObject::Multiply>("multiply")
      .InstanceMethod<&TestObject::Add>("add")
      .InstanceMethod<&TestObject::GetArgLength>("getArgLength")
      .InstanceAccessor<&TestObject::num>("readonlyNum")
      .InstanceAccessor<&TestObject::num, &TestObject::set_num>("num");

  naah::Registration::Class<AnotherTestObject>("AnotherTestObject")
      .Constructor<const Napi::CallbackInfo&>()
      .InstanceMethod<&AnotherTestObject::AddTest>("addTest")
      .InstanceMethod<&AnotherTestObject::GetTestNum>("getTestNum");
}

Napi::Object InitScriptWrappable(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  naah::Registration::Init(env, obj);

  return obj;
}
