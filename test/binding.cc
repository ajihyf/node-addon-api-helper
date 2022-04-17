#include <napi.h>

extern Napi::Object InitConvert(Napi::Env);

extern Napi::Object InitFunction(Napi::Env);

extern Napi::Object InitScriptWrappable(Napi::Env);

extern Napi::Object InitMultithread(Napi::Env);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports["function"] = InitFunction(env);
  exports["scriptWrappable"] = InitScriptWrappable(env);
  exports["convert"] = InitConvert(env);
  exports["multithread"] = InitMultithread(env);

  return exports;
}

NODE_API_MODULE(addon, Init)
