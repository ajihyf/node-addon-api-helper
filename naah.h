#ifndef SRC_NAAH_H_
#define SRC_NAAH_H_

#include <napi.h>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace naah {

template <typename T, typename Enabled = void>
struct ValueTransformer {
  static std::optional<T> FromJS(Napi::Value);

  static Napi::Value ToJS(Napi::Env, T);
};

template <typename Ret = void>
class AsyncWork {
 public:
  template <typename Arg>
  AsyncWork(Arg &&arg);

  std::function<Ret()> task;
};

template <typename T, typename E = std::nullptr_t>
class Result {
 public:
  Result() = delete;
  Result(T t);
  Result(E e);

  std::optional<T> value;
  std::optional<E> error;
};

template <typename T>
Napi::Value ConvertToJS(Napi::Env env, T v);

struct Undefined {};
struct Null {};

class ArrayBuffer : public std::vector<char> {
 private:
  using Super = std::vector<char>;

 public:
  using Super::Super;
};

template <typename E, napi_typedarray_type napi_type>
class TypedArrayOf : public std::vector<E> {
 private:
  using Super = std::vector<E>;

 public:
  using Super::Super;
};

using Uint8Array = TypedArrayOf<uint8_t, napi_uint8_array>;
using Uint8ClampedArray = TypedArrayOf<uint8_t, napi_uint8_clamped_array>;
using Int8Array = TypedArrayOf<int8_t, napi_int8_array>;
using Uint16Array = TypedArrayOf<uint16_t, napi_uint16_array>;
using Int16Array = TypedArrayOf<int16_t, napi_int16_array>;
using Uint32Array = TypedArrayOf<uint32_t, napi_uint32_array>;
using Int32Array = TypedArrayOf<int32_t, napi_int32_array>;
using Float32Array = TypedArrayOf<float, napi_float32_array>;
using Float64Array = TypedArrayOf<double, napi_float64_array>;
#if NAPI_VERSION > 5
using BigUint64Array = TypedArrayOf<uint64_t, napi_biguint64_array>;
using BigInt64Array = TypedArrayOf<int64_t, napi_bigint64_array>;
#endif

class Error
#ifdef NAPI_CPP_EXCEPTIONS
    : public std::exception
#endif  // NAPI_CPP_EXCEPTIONS
{
 public:
  using JSError = Napi::Error;
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
  using JSError = Napi::RangeError;
  using Error::Error;
};

class TypeError : public Error {
 public:
  using JSError = Napi::TypeError;
  using Error::Error;
};

struct ClassMetaInfo;
namespace details {
class ScriptWrappable;
}

class Class {
 private:
  ClassMetaInfo *_meta_info;

  friend class details::ScriptWrappable;

 public:
  using ConstructFn = std::unique_ptr<Class> (*)(const Napi::CallbackInfo &);

  Class();
  virtual ~Class();

  const ClassMetaInfo *meta_info() const;
};

struct ClassMetaInfo {
  const char *name;
  ClassMetaInfo *parent;
  Class::ConstructFn ctor;
  std::vector<napi_property_descriptor> descriptors;
};

template <typename T>
class ClassRegistration {
 public:
  ~ClassRegistration();

  template <typename... CtorArgs>
  ClassRegistration<T> &Constructor();

  template <typename P>
  ClassRegistration<T> &Inherits();

  template <auto T::*fn>
  ClassRegistration<T> &InstanceMethod(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*getter>
  ClassRegistration<T> &InstanceAccessor(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto T::*getter, auto T::*setter>
  ClassRegistration<T> &InstanceAccessor(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto fn>
  ClassRegistration<T> &StaticMethod(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto getter>
  ClassRegistration<T> &StaticAccessor(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

  template <auto getter, auto setter>
  ClassRegistration<T> &StaticAccessor(
      const char *name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);
};

struct Object {};

template <typename T>
class ObjectRegistration {
 public:
  template <auto T::*m>
  ObjectRegistration Member(const char *name);
};

class Registration : public Napi::Addon<Registration> {
 public:
  template <typename T>
  static void Value(const char *name, T t);

  template <auto fn>
  static void Function(const char *name, void *data = nullptr);

  template <typename Callable>
  static void Function(const char *name, Callable callable,
                       void *data = nullptr);

  template <typename T>
  static ClassRegistration<T> Class(const char *name);

  template <typename T>
  static ObjectRegistration<T> Object();

  Registration(Napi::Env env, Napi::Object exports);

  template <typename T>
  Napi::Function FindClass();

 private:
  std::map<ClassMetaInfo *, Napi::FunctionReference> classes_;
  void DefineClass(Napi::Env env, ClassMetaInfo *meta_info,
                   Napi::Object exports);
};

}  // namespace naah

#include "naah_inl.h"

#endif  // SRC_NAAH_H_
