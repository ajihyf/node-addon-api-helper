#include <napi.h>

extern Napi::Object InitFunction(Napi::Env);

extern Napi::Object InitScriptWrappable(Napi::Env);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports["function"] = InitFunction(env);
  exports["scriptWrappable"] = InitScriptWrappable(env);
  return exports;
}

NODE_API_MODULE(addon, Init)
