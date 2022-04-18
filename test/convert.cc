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
                            : std::nullopt,
  };
}

auto FnLambda(uint32_t num) {
  return [num = std::make_unique<uint32_t>(num)](uint32_t add) -> uint32_t {
    return *num + add;
  };
}

auto FnFunction(uint32_t num) {
  return std::function([num](uint32_t n) -> uint32_t { return num + n; });
}

auto FnVoidFunction(uint32_t) {
  return std::function([](uint32_t) -> void {});
}

NapiHelper::Error ErrorFunction() { return NapiHelper::Error("error"); }
NapiHelper::RangeError RangeErrorFunction() {
  return NapiHelper::RangeError("error");
}
NapiHelper::TypeError TypeErrorFunction() {
  return NapiHelper::TypeError("error");
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
    if (v.num.has_value()) {
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
  obj["num"] = NapiHelper::ConvertToJS(
      env, NapiHelper::ValueTransformer<uint32_t>::FromJS(num));

  obj["hehe"] = NapiHelper::ConvertToJS(env, "hehe");
  obj["stringView"] =
      NapiHelper::ConvertToJS(env, std::string_view("stringView"));
  obj["u16stringView"] =
      NapiHelper::ConvertToJS(env, std::u16string_view(u"u16stringView"));

  obj["customMethod"] = NapiHelper::Function::New<CustomMethod>(env);

  obj["fnLambda"] = NapiHelper::Function::New<FnLambda>(env);
  obj["fnFunction"] = NapiHelper::Function::New<FnFunction>(env);

  obj["errorFunction"] = NapiHelper::Function::New<ErrorFunction>(env);
  obj["rangeErrorFunction"] =
      NapiHelper::Function::New<RangeErrorFunction>(env);
  obj["typeErrorFunction"] = NapiHelper::Function::New<TypeErrorFunction>(env);
  obj["fnVoidFunction"] = NapiHelper::Function::New<FnVoidFunction>(env);

  return obj;
}
