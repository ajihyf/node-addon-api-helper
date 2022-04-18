#include <napi_helper.h>

namespace {
class TestObject : public NapiHelper::Class {
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

  static Napi::Function DefineClass(Napi::Env env) {
    using Wrapped = NapiHelper::ScriptWrappable<TestObject>;

    Napi::Symbol sym = Napi::Symbol::New(env, "sym");
    return Wrapped::DefineClass<uint32_t>(
        env, "TestObject",
        {Wrapped::StaticValue("sym", sym),
         Wrapped::StaticMethod<TestObject::AddStatic>("add"),
         Wrapped::StaticAccessor<TestObject::count>("readonlyCount"),
         Wrapped::StaticAccessor<TestObject::count, TestObject::set_count>(
             "count"),

         Wrapped::InstanceValue("iv", Napi::Number::New(env, 3154)),
         Wrapped::InstanceMethod<&TestObject::SymMethod>(sym),
         Wrapped::InstanceMethod<&TestObject::Multiply>("multiply"),
         Wrapped::InstanceMethod<&TestObject::Add>("add"),
         Wrapped::InstanceMethod<&TestObject::GetArgLength>("getArgLength"),
         Wrapped::InstanceAccessor<&TestObject::num>("readonlyNum"),
         Wrapped::InstanceAccessor<&TestObject::num, &TestObject::set_num>(
             "num")});
  }

 private:
  static uint32_t _count;
  uint32_t _num;
};

uint32_t TestObject::_count = 0;

class AnotherTestObject : public NapiHelper::Class {
 public:
  AnotherTestObject(const Napi::CallbackInfo&) {}

  void AddTest(TestObject* obj, uint32_t num) { obj->Add(num); }

  uint32_t GetTestNum(const TestObject* obj) { return obj->num(); }

  static Napi::Function DefineClass(Napi::Env env) {
    using Wrapped = NapiHelper::ScriptWrappable<AnotherTestObject>;

    return Wrapped::DefineClass<const Napi::CallbackInfo&>(
        env, "AnotherTestObject",
        {Wrapped::InstanceMethod<&AnotherTestObject::AddTest>("addTest"),
         Wrapped::InstanceMethod<&AnotherTestObject::GetTestNum>(
             "getTestNum")});
  }
};
}  // namespace

Napi::Object InitScriptWrappable(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  obj["TestObject"] = TestObject::DefineClass(env);
  obj["AnotherTestObject"] = AnotherTestObject::DefineClass(env);

  return obj;
}
