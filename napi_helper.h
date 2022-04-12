#ifndef SRC_NAPI_HELPER_H_
#define SRC_NAPI_HELPER_H_

#include <napi.h>

namespace NapiHelper {

class Error :
#ifdef NAPI_CPP_EXCEPTIONS
    public std::exception
#endif // NAPI_CPP_EXCEPTIONS
{
public:
  explicit Error(const char *msg);
  explicit Error(const std::string &msg);
#ifdef NAPI_CPP_EXCEPTIONS
  const char *what() const NAPI_NOEXCEPT override;
#endif // NAPI_CPP_EXCEPTIONS

  const std::string &Message() const NAPI_NOEXCEPT;

private:
  std::string _message;
};

class RangeError : public Error {
public:
  using Error::Error;
};

class TypeError : public Error {
public:
  using Error::Error;
};

class Function {
public:
  template <auto fn, const char *bad_arguments_message = nullptr>
  static Napi::Function New(Napi::Env env, const char *utf8name = nullptr,
                            void *data = nullptr);

  template <typename Callable>
  static Napi::Function
  New(Napi::Env env, Callable fn, const char *utf8name = nullptr,
      void *data = nullptr, const char *bad_arguments_message = nullptr);
};

} // namespace NapiHelper

#include "napi_helper_inl.h"

#endif // SRC_NAPI_HELPER_H_