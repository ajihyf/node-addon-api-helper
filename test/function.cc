#include <napi_helper.h>

namespace {
uint32_t ValueCallback(int32_t num1, std::optional<uint32_t> num2,
                       std::string str) {
  return num1 + num2.value_or(42) + str.size();
}

void VoidCallback(int32_t, std::optional<uint32_t>, std::string) {}

uint32_t ValueCallbackWithInfo(const Napi::CallbackInfo &info, int32_t num1) {
  return num1 + info.Length();
}
} // namespace

Napi::Object InitFunction(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  obj["valueCallback"] = NapiHelper::Function::New<ValueCallback>(env);

  obj["voidCallback"] = NapiHelper::Function::New<VoidCallback>(env);

  obj["valueCallbackWithInfo"] =
      NapiHelper::Function::New<ValueCallbackWithInfo>(env);

  return obj;
}