#ifndef SRC_NAPI_HELPER_H_
#define SRC_NAPI_HELPER_H_

#include <napi.h>

#if !defined(NAPI_HELPER_TAG_OBJECT_WRAP) && \
    !defined(NAPI_HELPER_DISABLE_TAG_OBJECT_WRAP)
#if NAPI_VERSION >= 8
#define NAPI_HELPER_TAG_OBJECT_WRAP
#endif
#endif

namespace NapiHelper {

template <typename T, typename Enabled = void>
struct ValueTransformer {
  static std::optional<T> FromJS(Napi::Value);

  static Napi::Value ToJS(Napi::Env, const T &);
};

struct Convert {
  template <typename T>
  static std::optional<T> FromJS(Napi::Value v);

  template <typename T>
  static Napi::Value ToJS(Napi::Env env, const T &v);
};

struct Undefined {};
struct Null {};

class Error :
#ifdef NAPI_CPP_EXCEPTIONS
    public std::exception
#endif  // NAPI_CPP_EXCEPTIONS
{
 public:
  explicit Error(const char *msg);
  explicit Error(const std::string &msg);
#ifdef NAPI_CPP_EXCEPTIONS
  const char *what() const NAPI_NOEXCEPT override;
#endif  // NAPI_CPP_EXCEPTIONS

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
  template <auto fn>
  static Napi::Function New(Napi::Env env, const char *utf8name = nullptr,
                            void *data = nullptr);

  template <auto fn>
  static Napi::Function New(Napi::Env env, const std::string &utf8name,
                            void *data = nullptr);

  template <typename Callable>
  static Napi::Function New(Napi::Env env, Callable fn,
                            const char *utf8name = nullptr,
                            void *data = nullptr);

  template <typename Callable>
  static Napi::Function New(Napi::Env env, Callable fn,
                            const std::string &utf8name, void *data = nullptr);
};

template <typename T>
class ScriptWrappable : public Napi::ObjectWrap<ScriptWrappable<T>> {
 private:
  using This = ScriptWrappable<T>;

  std::unique_ptr<T> _wrapped;

  template <auto T::*fn>
  auto InstanceMethodCallback(const Napi::CallbackInfo &);

  template <auto T::*fn>
  void InstanceSetterCallback(const Napi::CallbackInfo &, const Napi::Value &);

  template <auto fn>
  static auto StaticMethodCallback(const Napi::CallbackInfo &);

  template <auto fn>
  static void StaticSetterCallback(const Napi::CallbackInfo &,
                                   const Napi::Value &);

 public:
  ScriptWrappable(const Napi::CallbackInfo &info);

  T &wrapped() const;

  using PropertyDescriptor =
      typename Napi::ObjectWrap<This>::PropertyDescriptor;

  template <typename... CtorArgs>
  static Napi::Function DefineClass(
      Napi::Env env, const char *utf8name,
      const std::initializer_list<PropertyDescriptor> &properties);

  template <typename... CtorArgs>
  static Napi::Function DefineClass(
      Napi::Env env, const char *utf8name,
      const std::vector<PropertyDescriptor> &properties);

  template <auto T::*fn>
  static PropertyDescriptor InstanceMethod(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*fn>
  static PropertyDescriptor InstanceMethod(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*getter>
  static PropertyDescriptor InstanceAccessor(
      const char *utf8name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*getter, auto T::*setter>
  static PropertyDescriptor InstanceAccessor(
      const char *utf8name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*getter>
  static PropertyDescriptor InstanceAccessor(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*getter, auto T::*setter>
  static PropertyDescriptor InstanceAccessor(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto fn>
  static PropertyDescriptor StaticMethod(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto fn>
  static PropertyDescriptor StaticMethod(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto getter>
  static PropertyDescriptor StaticAccessor(
      const char *utf8name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto getter, auto setter>
  static PropertyDescriptor StaticAccessor(
      const char *utf8name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto getter>
  static PropertyDescriptor StaticAccessor(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto getter, auto setter = nullptr>
  static PropertyDescriptor StaticAccessor(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

#ifdef NAPI_HELPER_TAG_OBJECT_WRAP
  static const napi_type_tag *type_tag();
#endif
};

}  // namespace NapiHelper

#include "napi_helper_inl.h"

#endif  // SRC_NAPI_HELPER_H_
