#include <naah.h>

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

naah::Error ErrorFunction() { return naah::Error("error"); }
naah::RangeError RangeErrorFunction() { return naah::RangeError("error"); }
naah::TypeError TypeErrorFunction() { return naah::TypeError("error"); }
}  // namespace

namespace naah {
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
}  // namespace naah

Napi::Object InitConvert(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  Napi::Number num = Napi::Number::New(env, 42);
  obj["num"] =
      naah::ConvertToJS(env, naah::ValueTransformer<uint32_t>::FromJS(num));

  obj["hehe"] = naah::ConvertToJS(env, "hehe");
  obj["stringView"] = naah::ConvertToJS(env, std::string_view("stringView"));
  obj["u16stringView"] =
      naah::ConvertToJS(env, std::u16string_view(u"u16stringView"));

  obj["customMethod"] = naah::details::Function::New<CustomMethod>(env);

  obj["fnLambda"] = naah::details::Function::New<FnLambda>(env);
  obj["fnFunction"] = naah::details::Function::New<FnFunction>(env);

  obj["errorFunction"] = naah::details::Function::New<ErrorFunction>(env);
  obj["rangeErrorFunction"] =
      naah::details::Function::New<RangeErrorFunction>(env);
  obj["typeErrorFunction"] =
      naah::details::Function::New<TypeErrorFunction>(env);
  obj["fnVoidFunction"] = naah::details::Function::New<FnVoidFunction>(env);

  return obj;
}
