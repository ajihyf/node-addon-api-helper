#include <napi.h>

extern Napi::Object InitFunction(Napi::Env);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports["function"] = InitFunction(env);
  return exports;
}

NODE_API_MODULE(addon, Init)