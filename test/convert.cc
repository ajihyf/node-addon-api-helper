#include <napi_helper.h>

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

Napi::Object InitConvert(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  Napi::Number num = Napi::Number::New(env, 42);
  obj["num"] = NapiHelper::Convert::ToJS(
      env, NapiHelper::Convert::FromJS<uint32_t>(num));

  obj["hehe"] = NapiHelper::Convert::ToJS(env, "hehe");
  obj["stringView"] =
      NapiHelper::Convert::ToJS(env, std::string_view("stringView"));
  obj["u16stringView"] =
      NapiHelper::Convert::ToJS(env, std::u16string_view(u"u16stringView"));

  obj["customMethod"] = NapiHelper::Function::New<CustomMethod>(env);
  return obj;
}