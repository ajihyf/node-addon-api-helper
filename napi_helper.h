#ifndef SRC_NAPI_HELPER_H_
#define SRC_NAPI_HELPER_H_

#include <napi.h>

namespace NapiHelper {

class Function {
public:
  template <auto fn>
  static Napi::Function New(Napi::Env env, const char *utf8name = nullptr,
                            void *data = nullptr);
};

} // namespace NapiHelper

#include "napi_helper_inl.h"

#endif // SRC_NAPI_HELPER_H_