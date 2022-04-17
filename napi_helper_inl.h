#ifndef SRC_NAPI_HELPER_INL_H_
#define SRC_NAPI_HELPER_INL_H_

#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

namespace NapiHelper {

namespace details {

template <typename T, typename Enable = void>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <class T>
struct remove_optional {
  typedef T type;
};
template <class T>
struct remove_optional<std::optional<T>> {
  typedef T type;
};

// inspired by https://stackoverflow.com/a/12283159
template <typename T>
struct get_signature : public get_signature<decltype(&T::operator())> {};

template <typename C, typename R, typename... Args>
struct get_signature<R (C::*)(Args...) const> {
  typedef R ret;
  typedef std::tuple<Args...> args;
};

template <typename C, typename R, typename... Args>
struct get_signature<R (C::*)(Args...)> {
  typedef R ret;
  typedef std::tuple<Args...> args;
};

template <typename R, typename... Args>
struct get_signature<R (*)(Args...)> {
  typedef R ret;
  typedef std::tuple<Args...> args;
};

template <typename R, typename... Args>
struct get_signature<R(Args...)> {
  typedef R ret;
  typedef std::tuple<Args...> args;
};

template <typename T>
struct get_tuple_elements;

template <>
struct get_tuple_elements<std::tuple<>> {
  typedef void head;
  typedef std::tuple<> rest;
};

template <typename Head, typename... Rest>
struct get_tuple_elements<std::tuple<Head, Rest...>> {
  typedef Head head;
  typedef std::tuple<Rest...> rest;
};

template <typename T>
class ArgsConverter;

template <>
class ArgsConverter<std::tuple<>> {
 public:
  template <typename Args>
  static std::optional<std::tuple<>> Get(Args &&, size_t) {
    return std::tuple<>();
  }
};

template <typename Head, typename... Rest>
class ArgsConverter<std::tuple<Head, Rest...>> {
 public:
  template <typename Args>
  static std::optional<std::tuple<Head, Rest...>> Get(Args &&args, size_t i) {
    using Real = typename remove_optional<Head>::type;

    std::optional<Real> opt_head = ValueTransformer<Real>::FromJS(args[i]);

    if constexpr (!is_optional<Head>::value) {
      if (!opt_head.has_value()) {
        return {};
      }
    }

    std::optional<std::tuple<Rest...>> rest =
        ArgsConverter<std::tuple<Rest...>>::Get(std::forward<Args>(args),
                                                i + 1);
    if (!rest.has_value()) {
      return {};
    }

    if constexpr (is_optional<Head>::value) {
      return std::tuple_cat(std::make_tuple(std::move(opt_head)),
                            std::move(*rest));
    } else {
      return std::tuple_cat(std::make_tuple(std::move(*opt_head)),
                            std::move(*rest));
    }
  }
};

}  // namespace details

template <>
struct ValueTransformer<Undefined> {
  static std::optional<Undefined> FromJS(Napi::Value v) {
    if (!v.IsUndefined()) {
      return {};
    }
    return Undefined{};
  }
  static Napi::Value ToJS(Napi::Env env, Undefined) { return env.Undefined(); }
};

template <>
struct ValueTransformer<Null> {
  static std::optional<Null> FromJS(Napi::Value v) {
    if (!v.IsNull()) {
      return {};
    }
    return Null{};
  }
  static Napi::Value ToJS(Napi::Env env, Null) { return env.Null(); }
};

template <>
struct ValueTransformer<Napi::Value> {
  static std::optional<Napi::Value> FromJS(Napi::Value v) { return v; }
  static Napi::Value ToJS(Napi::Env, Napi::Value val) { return val; }
};

template <typename T, bool (Napi::Value::*IsT)() const>
struct JSValueTransformer {
  static std::optional<T> FromJS(Napi::Value value) {
    if (!(value.*IsT)()) {
      return {};
    }
    return value.As<T>();
  }

  static Napi::Value ToJS(Napi::Env, T val) { return val; }
};

template <>
struct ValueTransformer<Napi::Boolean>
    : public JSValueTransformer<Napi::Boolean, &Napi::Value::IsBoolean> {};

template <>
struct ValueTransformer<Napi::Number>
    : public JSValueTransformer<Napi::Number, &Napi::Value::IsNumber> {};

#if NAPI_VERSION > 5
template <>
struct ValueTransformer<Napi::BigInt>
    : public JSValueTransformer<Napi::BigInt, &Napi::Value::IsBigInt> {};
#endif

#if (NAPI_VERSION > 4)
template <>
struct ValueTransformer<Napi::Date>
    : public JSValueTransformer<Napi::Date, &Napi::Value::IsDate> {};
#endif

template <>
struct ValueTransformer<Napi::String>
    : public JSValueTransformer<Napi::String, &Napi::Value::IsString> {};

template <>
struct ValueTransformer<Napi::Symbol>
    : public JSValueTransformer<Napi::Symbol, &Napi::Value::IsSymbol> {};

template <>
struct ValueTransformer<Napi::Array>
    : public JSValueTransformer<Napi::Array, &Napi::Value::IsArray> {};

template <>
struct ValueTransformer<Napi::ArrayBuffer>
    : public JSValueTransformer<Napi::ArrayBuffer,
                                &Napi::Value::IsArrayBuffer> {};

template <>
struct ValueTransformer<Napi::TypedArray>
    : public JSValueTransformer<Napi::TypedArray, &Napi::Value::IsTypedArray> {
};

template <>
struct ValueTransformer<Napi::Object>
    : public JSValueTransformer<Napi::Object, &Napi::Value::IsObject> {};

template <>
struct ValueTransformer<Napi::Function>
    : public JSValueTransformer<Napi::Function, &Napi::Value::IsFunction> {};

template <>
struct ValueTransformer<Napi::Promise>
    : public JSValueTransformer<Napi::Promise, &Napi::Value::IsPromise> {};

template <>
struct ValueTransformer<Napi::DataView>
    : public JSValueTransformer<Napi::DataView, &Napi::Value::IsDataView> {};

template <typename T>
struct ValueTransformer<Napi::Buffer<T>>
    : public JSValueTransformer<Napi::Buffer<T>, &Napi::Value::IsBuffer> {};

template <typename T>
struct ValueTransformer<Napi::External<T>>
    : public JSValueTransformer<Napi::External<T>, &Napi::Value::IsExternal> {};

template <typename T, napi_typedarray_type type>
struct TypedArrayTransformer {
  static std::optional<T> FromJS(Napi::Value value) {
    if (!value.IsTypedArray()) {
      return {};
    }
    Napi::TypedArray arr = value.As<Napi::TypedArray>();
    if (arr.TypedArrayType() != type) {
      return {};
    }
    return value.As<T>();
  }

  static Napi::Value ToJS(Napi::Env, T val) { return val; }
};

template <>
struct ValueTransformer<Napi::Int8Array>
    : public TypedArrayTransformer<Napi::Int8Array, napi_int8_array> {};

template <>
struct ValueTransformer<Napi::Uint8Array>
    : public TypedArrayTransformer<Napi::Uint8Array, napi_uint8_array> {};

template <>
struct ValueTransformer<Napi::Int16Array>
    : public TypedArrayTransformer<Napi::Int16Array, napi_int16_array> {};

template <>
struct ValueTransformer<Napi::Uint16Array>
    : public TypedArrayTransformer<Napi::Uint16Array, napi_uint16_array> {};

template <>
struct ValueTransformer<Napi::Int32Array>
    : public TypedArrayTransformer<Napi::Int32Array, napi_int32_array> {};

template <>
struct ValueTransformer<Napi::Uint32Array>
    : public TypedArrayTransformer<Napi::Uint32Array, napi_uint32_array> {};

template <>
struct ValueTransformer<Napi::Float32Array>
    : public TypedArrayTransformer<Napi::Float32Array, napi_float32_array> {};

template <>
struct ValueTransformer<Napi::Float64Array>
    : public TypedArrayTransformer<Napi::Float64Array, napi_float64_array> {};

#if NAPI_VERSION > 5
template <>
struct ValueTransformer<Napi::BigInt64Array>
    : public TypedArrayTransformer<Napi::BigInt64Array, napi_bigint64_array> {};

template <>
struct ValueTransformer<Napi::BigUint64Array>
    : public TypedArrayTransformer<Napi::BigUint64Array, napi_biguint64_array> {
};
#endif

template <>
struct ValueTransformer<bool> {
  static std::optional<bool> FromJS(Napi::Value value) {
    if (!value.IsBoolean()) {
      return {};
    }
    return value.As<Napi::Boolean>().Value();
  }

  static Napi::Value ToJS(Napi::Env env, bool num) {
    return Napi::Boolean::New(env, num);
  }
};

template <>
struct ValueTransformer<double> {
  static std::optional<double> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().DoubleValue();
  }

  static Napi::Value ToJS(Napi::Env env, double num) {
    return Napi::Number::New(env, num);
  }
};

template <>
struct ValueTransformer<float> {
  static std::optional<float> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().FloatValue();
  }

  static Napi::Value ToJS(Napi::Env env, float num) {
    return Napi::Number::New(env, num);
  }
};

template <typename Uint>
struct ValueTransformer<Uint,
                        std::enable_if_t<std::is_same_v<Uint, uint8_t> ||
                                         std::is_same_v<Uint, uint16_t> ||
                                         std::is_same_v<Uint, uint32_t>>> {
  static std::optional<Uint> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().Uint32Value();
  }

  static Napi::Value ToJS(Napi::Env env, Uint num) {
    return Napi::Number::New(env, num);
  }
};

template <typename Int>
struct ValueTransformer<Int, std::enable_if_t<std::is_same_v<Int, int8_t> ||
                                              std::is_same_v<Int, int16_t> ||
                                              std::is_same_v<Int, int32_t>>> {
  static std::optional<Int> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().Int32Value();
  }

  static Napi::Value ToJS(Napi::Env env, Int num) {
    return Napi::Number::New(env, num);
  }
};

#if NAPI_VERSION > 5
template <>
struct ValueTransformer<int64_t> {
  static std::optional<int64_t> FromJS(Napi::Value value) {
    if (!value.IsBigInt()) {
      return {};
    }
    bool lossless = true;
    return value.As<Napi::BigInt>().Int64Value(&lossless);
  }

  static Napi::Value ToJS(Napi::Env env, int64_t num) {
    return Napi::BigInt::New(env, num);
  }
};

template <>
struct ValueTransformer<uint64_t> {
  static std::optional<uint64_t> FromJS(Napi::Value value) {
    if (!value.IsBigInt()) {
      return {};
    }
    bool lossless = true;
    return value.As<Napi::BigInt>().Uint64Value(&lossless);
  }

  static Napi::Value ToJS(Napi::Env env, uint64_t num) {
    return Napi::BigInt::New(env, num);
  }
};
#endif

template <typename T>
struct ValueTransformer<
    T, std::enable_if_t<std::is_convertible_v<T, const char *>>> {
 public:
  static Napi::Value ToJS(Napi::Env env, T str) {
    return Napi::String::New(env, str);
  }
};

template <>
struct ValueTransformer<std::string_view> {
  static Napi::Value ToJS(Napi::Env env, std::string_view str) {
    return Napi::String::New(env, str.data(), str.size());
  }
};

template <>
struct ValueTransformer<std::u16string_view> {
  static Napi::Value ToJS(Napi::Env env, std::u16string_view str) {
    return Napi::String::New(env, str.data(), str.size());
  }
};

template <>
struct ValueTransformer<std::string> {
  static std::optional<std::string> FromJS(Napi::Value value) {
    if (!value.IsString()) {
      return {};
    }
    return value.As<Napi::String>().Utf8Value();
  }

  static Napi::Value ToJS(Napi::Env env, std::string str) {
    return Napi::String::New(env, str);
  }
};

template <>
struct ValueTransformer<std::u16string> {
  static std::optional<std::u16string> FromJS(Napi::Value value) {
    if (!value.IsString()) {
      return {};
    }
    return value.As<Napi::String>().Utf16Value();
  }

  static Napi::Value ToJS(Napi::Env env, std::u16string str) {
    return Napi::String::New(env, str);
  }
};

template <typename T>
struct ValueTransformer<std::optional<T>> {
  static Napi::Value ToJS(Napi::Env env, std::optional<T> val) {
    if (val.has_value()) {
      return ValueTransformer<T>::ToJS(env, std::move(*val));
    }
    return env.Undefined();
  }
};

template <typename... Args>
struct ValueTransformer<std::variant<Args...>> {
 private:
  using Variants = std::variant<Args...>;

  template <size_t I>
  static std::optional<Variants> FromJS([[maybe_unused]] Napi::Value value) {
    if constexpr (I < sizeof...(Args)) {
      using TypeI = std::variant_alternative_t<I, Variants>;
      std::optional<TypeI> result = ValueTransformer<TypeI>::FromJS(value);
      if (result.has_value()) {
        return std::move(*result);
      }
      return FromJS<I + 1>(value);
    } else {
      return {};
    }
  }

 public:
  static std::optional<Variants> FromJS(Napi::Value value) {
    return FromJS<0>(value);
  }

  static Napi::Value ToJS(Napi::Env env, Variants v) {
    return std::visit(
        [&](auto &&arg) -> Napi::Value {
          using T = std::decay_t<decltype(arg)>;
          return ValueTransformer<T>::ToJS(env, std::move(arg));
        },
        v);
  }
};

template <typename Callable>
struct ValueTransformer<
    Callable,
    std::enable_if_t<
        std::is_function_v<std::remove_pointer_t<Callable>> ||
        std::is_member_function_pointer_v<decltype(&Callable::operator())>>> {
  static Napi::Value ToJS(Napi::Env env, Callable v) {
    return Function::New(env, std::move(v));
  }
};

template <typename... Args>
struct ValueTransformer<std::tuple<Args...>> {
 private:
  using Tuple = std::tuple<Args...>;

  template <size_t... Is>
  static void SetArrayElement(Napi::Env env, Napi::Array &arr, Tuple t,
                              std::index_sequence<Is...>) {
    (arr.Set(static_cast<uint32_t>(Is),
             ValueTransformer<std::tuple_element_t<Is, Tuple>>::ToJS(
                 env, std::move(std::get<Is>(t)))),
     ...);
  }

 public:
  static std::optional<Tuple> FromJS(Napi::Value value) {
    if (!value.IsArray()) {
      return {};
    }
    return details::ArgsConverter<Tuple>::Get(value.As<Napi::Array>(), 0);
  }

  static Napi::Value ToJS(Napi::Env env, Tuple t) {
    return std::apply(
        [&](auto &&...elements) -> Napi::Array {
          constexpr size_t size = sizeof...(elements);

          Napi::Array result = Napi::Array::New(env, size);

          SetArrayElement(env, result, std::move(t),
                          std::make_index_sequence<size>());

          return result;
        },
        t);
  }
};

template <typename T>
struct ValueTransformer<std::vector<T>> {
  static std::optional<std::vector<T>> FromJS(Napi::Value value) {
    if (!value.IsArray()) {
      return {};
    }
    Napi::Array arr = value.As<Napi::Array>();
    uint32_t len = arr.Length();
    std::vector<T> result;
    result.reserve(len);
    for (uint32_t i = 0; i < len; i++) {
      std::optional<T> item = ValueTransformer<T>::FromJS(arr.Get(i));
      if (!item.has_value()) {
        return {};
      }
      result.push_back(std::move(*item));
    }
    return std::move(result);
  }

  static Napi::Value ToJS(Napi::Env env, std::vector<T> arr) {
    Napi::Array result = Napi::Array::New(env, arr.size());
    for (uint32_t i = 0; i < arr.size(); i++) {
      result.Set(i, ValueTransformer<T>::ToJS(env, std::move(arr[i])));
    }
    return result;
  }
};

#ifdef NAPI_HELPER_TAG_OBJECT_WRAP

template <typename T>
struct ValueTransformer<T *, std::enable_if_t<std::is_class_v<T>>> {
 public:
  static std::optional<T *> FromJS(Napi::Value value) {
    using Wrapped = ScriptWrappable<std::remove_const_t<T>>;
    if (!value.IsObject()) {
      return {};
    }
    Napi::Object obj = value.As<Napi::Object>();
    bool check_tag = false;
    napi_check_object_type_tag(value.Env(), obj, Wrapped::type_tag(),
                               &check_tag);
    if (!check_tag) {
      return {};
    }
    return &(Wrapped::Unwrap(obj)->wrapped());
  }
};

#endif

template <typename E>
struct ValueTransformer<
    E, std::enable_if_t<std::is_base_of_v<NapiHelper::Error, E>>> {
 public:
  static Napi::Value ToJS(Napi::Env env, E e) {
    return E::JSError::New(env, e.Message()).Value();
  }
};

template <typename T>
inline std::optional<T> Convert::FromJS(Napi::Value v) {
  return ValueTransformer<T>::FromJS(v);
}

template <typename T>
inline Napi::Value Convert::ToJS(Napi::Env env, T v) {
  return ValueTransformer<T>::ToJS(env, std::move(v));
}

inline Error::Error(const char *msg) : _message(msg) {}
inline Error::Error(const std::string &msg) : _message(msg) {}

inline const std::string &Error::Message() const NAPI_NOEXCEPT {
  return _message;
}

#ifdef NAPI_CPP_EXCEPTIONS

inline const char *Error::what() const NAPI_NOEXCEPT {
  return _message.c_str();
}

#endif  // NAPI_CPP_EXCEPTIONS

namespace details {
class Invoker {
 private:
  template <typename Callable>
  static auto CallInternal(const Napi::CallbackInfo &info, Callable &&fn) {
    using Signature = get_signature<std::decay_t<Callable>>;
    using OriginArgs = typename Signature::args;

    constexpr bool head_is_cb_info =
        std::is_same_v<typename get_tuple_elements<OriginArgs>::head,
                       const Napi::CallbackInfo &>;

    using Args = typename std::conditional_t<
        head_is_cb_info, typename get_tuple_elements<OriginArgs>::rest,
        OriginArgs>;

    std::optional<Args> args = ArgsConverter<Args>::Get(info, 0);
    if (!args.has_value()) {
      NAPI_THROW(Napi::TypeError::New(info.Env(), "bad arguments"),
                 typename Signature::ret());
    }

    if constexpr (head_is_cb_info) {
      return std::apply(
          std::forward<Callable>(fn),
          std::tuple_cat(std::make_tuple(std::cref(info)), std::move(*args)));
    } else {
      return std::apply(std::forward<Callable>(fn), std::move(*args));
    }
  }

 public:
  template <typename Callable>
  static auto Call(const Napi::CallbackInfo &info, Callable &&fn) {
#ifdef NAPI_CPP_EXCEPTIONS
    try {
      return CallInternal(info, std::forward<Callable>(fn));
    } catch (const RangeError &err) {
      throw RangeError::JSError::New(info.Env(), err.Message());
    } catch (const TypeError &err) {
      throw TypeError::JSError::New(info.Env(), err.Message());
    } catch (const Error &err) {
      throw Error::JSError::New(info.Env(), err.Message());
    }
#else
    return CallInternal(info, std::forward<Callable>(fn));
#endif
  }

  template <typename Callable>
  static auto CallJS(const Napi::CallbackInfo &info, Callable &&fn) {
    using Signature = get_signature<std::decay_t<Callable>>;
    using Ret = typename Signature::ret;

    if constexpr (std::is_void_v<Ret>) {
      return Call(info, std::forward<Callable>(fn));
    } else {
      Ret result = Call(info, std::forward<Callable>(fn));
      return ValueTransformer<Ret>::ToJS(info.Env(), std::move(result));
    }
  }

  template <auto fn>
  static auto FunctionCallback(const Napi::CallbackInfo &info) {
    return CallJS(info, fn);
  }

  template <typename T>
  using ConstructFn = std::unique_ptr<T> (*)(const Napi::CallbackInfo &);

  template <typename T, typename... CtorArgs>
  static std::unique_ptr<T> Construct(const Napi::CallbackInfo &info) {
    return Call(info, [](CtorArgs... args) -> std::unique_ptr<T> {
      return std::unique_ptr<T>(new T(std::move(args)...));
    });
  }

  template <class T, typename Ret, typename... Args>
  static auto InstanceCall(T *c, Ret (T::*m)(Args...)) {
    return [=](Args... args) -> Ret { return (c->*m)(std::move(args)...); };
  }

  template <class T, typename Ret, typename... Args>
  static auto InstanceCall(T *c, Ret (T::*m)(Args...) const) {
    return [=](Args... args) -> Ret { return (c->*m)(std::move(args)...); };
  }
};
}  // namespace details

template <auto fn>
inline Napi::Function Function::New(Napi::Env env, const char *utf8name,
                                    void *data) {
  return Napi::Function::New<details::Invoker::FunctionCallback<fn>>(
      env, utf8name, data);
}

template <auto fn>
inline Napi::Function Function::New(Napi::Env env, const std::string &utf8name,
                                    void *data) {
  return New<fn>(env, utf8name.c_str(), data);
}

template <typename Callable>
inline Napi::Function Function::New(Napi::Env env, Callable fn,
                                    const char *utf8name, void *data) {
  return Napi::Function::New(
      env,
      [fn = std::move(fn)](const Napi::CallbackInfo &info) -> auto {
        return details::Invoker::CallJS(info, fn);
      },
      utf8name, data);
}

template <typename Callable>
inline Napi::Function Function::New(Napi::Env env, Callable fn,
                                    const std::string &utf8name, void *data) {
  return New(env, fn, utf8name.c_str(), data);
}

template <typename T>
inline ScriptWrappable<T>::ScriptWrappable(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<This>(info) {
#ifdef NAPI_HELPER_TAG_OBJECT_WRAP
  napi_type_tag_object(info.Env(), info.This(), type_tag());
#endif
  auto t_ctor = reinterpret_cast<details::Invoker::ConstructFn<T>>(info.Data());

  _wrapped = t_ctor(info);
}

template <typename T>
inline T &ScriptWrappable<T>::wrapped() const {
  return *_wrapped;
}

template <typename T>
template <typename... CtorArgs>
Napi::Function ScriptWrappable<T>::DefineClass(
    Napi::Env env, const char *utf8name,
    const std::initializer_list<PropertyDescriptor> &properties) {
  using Wrapped = Napi::ObjectWrap<This>;

  details::Invoker::ConstructFn<T> fn =
      details::Invoker::Construct<T, CtorArgs...>;
  return Wrapped::DefineClass(env, utf8name, properties,
                              reinterpret_cast<void *>(fn));
}

template <typename T>
template <typename... CtorArgs>
Napi::Function ScriptWrappable<T>::DefineClass(
    Napi::Env env, const char *utf8name,
    const std::vector<PropertyDescriptor> &properties) {
  using Wrapped = Napi::ObjectWrap<This>;

  details::Invoker::ConstructFn<T> fn =
      details::Invoker::Construct<T, CtorArgs...>;

  return Wrapped::DefineClass(env, utf8name, properties,
                              reinterpret_cast<void *>(fn));
}

template <typename T>
template <auto T::*fn>
inline auto ScriptWrappable<T>::InstanceMethodCallback(
    const Napi::CallbackInfo &info) {
  return details::Invoker::CallJS(
      info, details::Invoker::InstanceCall(_wrapped.get(), fn));
}

template <typename T>
template <auto T::*fn>
inline void ScriptWrappable<T>::InstanceSetterCallback(
    const Napi::CallbackInfo &info, const Napi::Value &) {
  details::Invoker::CallJS(info,
                           details::Invoker::InstanceCall(_wrapped.get(), fn));
}

template <typename T>
template <auto T::*fn>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::InstanceMethod(const char *name,
                                   napi_property_attributes attributes,
                                   void *data) {
  return Napi::ObjectWrap<This>::template InstanceMethod<
      &This::InstanceMethodCallback<fn>>(name, attributes, data);
}

template <typename T>
template <auto T::*getter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::InstanceAccessor(const char *name,
                                     napi_property_attributes attributes,
                                     void *data) {
  return Napi::ObjectWrap<This>::template InstanceAccessor<
      &This::InstanceMethodCallback<getter>>(name, attributes, data);
}

template <typename T>
template <auto T::*getter, auto T::*setter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::InstanceAccessor(const char *name,
                                     napi_property_attributes attributes,
                                     void *data) {
  return Napi::ObjectWrap<This>::template InstanceAccessor<
      &This::InstanceMethodCallback<getter>,
      &This::InstanceSetterCallback<setter>>(name, attributes, data);
}

template <typename T>
template <auto T::*fn>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::InstanceMethod(Napi::Symbol name,
                                   napi_property_attributes attributes,
                                   void *data) {
  return Napi::ObjectWrap<This>::template InstanceMethod<
      &This::InstanceMethodCallback<fn>>(name, attributes, data);
}

template <typename T>
template <auto T::*getter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::InstanceAccessor(Napi::Symbol name,
                                     napi_property_attributes attributes,
                                     void *data) {
  return Napi::ObjectWrap<This>::template InstanceAccessor<
      &This::InstanceMethodCallback<getter>>(name, attributes, data);
}

template <typename T>
template <auto T::*getter, auto T::*setter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::InstanceAccessor(Napi::Symbol name,
                                     napi_property_attributes attributes,
                                     void *data) {
  return Napi::ObjectWrap<This>::template InstanceAccessor<
      &This::InstanceMethodCallback<getter>,
      &This::InstanceSetterCallback<setter>>(name, attributes, data);
}

template <typename T>
template <auto fn>
inline auto ScriptWrappable<T>::StaticMethodCallback(
    const Napi::CallbackInfo &info) {
  return details::Invoker::CallJS(info, fn);
}

template <typename T>
template <auto fn>
inline void ScriptWrappable<T>::StaticSetterCallback(
    const Napi::CallbackInfo &info, const Napi::Value &) {
  details::Invoker::CallJS(info, fn);
}

template <typename T>
template <auto fn>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::StaticMethod(const char *name,
                                 napi_property_attributes attributes,
                                 void *data) {
  return Napi::ObjectWrap<This>::template StaticMethod<
      &This::StaticMethodCallback<fn>>(name, attributes, data);
}

template <typename T>
template <auto getter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::StaticAccessor(const char *name,
                                   napi_property_attributes attributes,
                                   void *data) {
  return Napi::ObjectWrap<This>::template StaticAccessor<
      &This::StaticMethodCallback<getter>>(name, attributes, data);
}

template <typename T>
template <auto getter, auto setter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::StaticAccessor(const char *name,
                                   napi_property_attributes attributes,
                                   void *data) {
  return Napi::ObjectWrap<This>::template StaticAccessor<
      &This::StaticMethodCallback<getter>, &This::StaticSetterCallback<setter>>(
      name, attributes, data);
}

template <typename T>
template <auto fn>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::StaticMethod(Napi::Symbol name,
                                 napi_property_attributes attributes,
                                 void *data) {
  return Napi::ObjectWrap<This>::template StaticMethod<
      &This::StaticMethodCallback<fn>>(name, attributes, data);
}

template <typename T>
template <auto getter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::StaticAccessor(Napi::Symbol name,
                                   napi_property_attributes attributes,
                                   void *data) {
  return Napi::ObjectWrap<This>::template StaticAccessor<
      &This::StaticMethodCallback<getter>>(name, attributes, data);
}

template <typename T>
template <auto getter, auto setter>
inline typename ScriptWrappable<T>::PropertyDescriptor
ScriptWrappable<T>::StaticAccessor(Napi::Symbol name,
                                   napi_property_attributes attributes,
                                   void *data) {
  return Napi::ObjectWrap<This>::template StaticAccessor<
      &This::StaticMethodCallback<getter>, &This::StaticSetterCallback<setter>>(
      name, attributes, data);
}

#ifdef NAPI_HELPER_TAG_OBJECT_WRAP

template <typename T>
inline const napi_type_tag *ScriptWrappable<T>::type_tag() {
  static_assert(std::is_class_v<T>, "T must be class");
  static_assert(!std::is_const_v<T>,
                "T must be non const class");  // make sure no `const T` exists
  static const napi_type_tag tag = {reinterpret_cast<uintptr_t>(&tag),
                                    reinterpret_cast<uintptr_t>(&tag)};
  return &tag;
}

#endif

namespace details {
struct RegistrationEntry {
  const char *name;
  std::function<Napi::Value(Napi::Env, const char *)> init_cb;

  static std::vector<RegistrationEntry> &Entries() {
    static std::vector<RegistrationEntry> entries;
    return entries;
  }
};

template <typename T>
class ClassRegistrationEntry {
 private:
  static std::vector<typename ScriptWrappable<T>::PropertyDescriptor>
      &descriptors() {
    static std::vector<typename ScriptWrappable<T>::PropertyDescriptor>
        descriptors_;
    return descriptors_;
  }

  template <typename... CtorArgs>
  static Napi::Value DefineClassCallback(Napi::Env env, const char *name) {
    return ScriptWrappable<T>::template DefineClass<CtorArgs...>(env, name,
                                                                 descriptors());
  }

 public:
  template <typename... CtorArgs>
  static void DefineClass(const char *name) {
    RegistrationEntry::Entries().push_back(
        RegistrationEntry{name, DefineClassCallback<CtorArgs...>});
  }

  static void AddPropertyDescriptor(
      typename ScriptWrappable<T>::PropertyDescriptor descriptor) {
    descriptors().push_back(std::move(descriptor));
  }
};
}  // namespace details

inline Napi::Object Registration::ModuleCallback(Napi::Env env,
                                                 Napi::Object exports) {
  for (auto &it : details::RegistrationEntry::Entries()) {
    exports.Set(it.name, it.init_cb(env, it.name));
  }
  return exports;
}

template <typename T>
inline void Registration::Value(const char *name, T val) {
  details::RegistrationEntry::Entries().push_back(
      {name, [val = std::move(val)](Napi::Env env, const char *) {
         return ValueTransformer<T>::ToJS(env, std::move(val));
       }});
}

template <auto fn>
inline void Registration::Function(const char *name, void *data) {
  details::RegistrationEntry::Entries().push_back(
      {name, [data](Napi::Env env, const char *name) {
         return Function::New<fn>(env, name, data);
       }});
}

template <typename Callable>
inline void Registration::Function(const char *name, Callable callable,
                                   void *data) {
  details::RegistrationEntry::Entries().push_back(
      {name,
       [callable = std::move(callable), data](Napi::Env env, const char *name) {
         return Function::New(env, callable, name, data);
       }});
}

template <typename T, typename... CtorArgs>
inline ClassRegistration<T> Registration::Class(const char *name) {
  details::ClassRegistrationEntry<T>::template DefineClass<CtorArgs...>(name);
  return ClassRegistration<T>();
}

template <typename T>
template <auto T::*fn>
inline ClassRegistration<T> &ClassRegistration<T>::InstanceMethod(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistrationEntry<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template InstanceMethod<fn>(name, attributes, data));
  return *this;
}

template <typename T>
template <auto T::*getter>
inline ClassRegistration<T> &ClassRegistration<T>::InstanceAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistrationEntry<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template InstanceAccessor<getter>(name, attributes,
                                                            data));
  return *this;
}

template <typename T>
template <auto T::*getter, auto T::*setter>
inline ClassRegistration<T> &ClassRegistration<T>::InstanceAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistrationEntry<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template InstanceAccessor<getter, setter>(
          name, attributes, data));
  return *this;
}

template <typename T>
template <auto fn>
inline ClassRegistration<T> &ClassRegistration<T>::StaticMethod(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistrationEntry<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template StaticMethod<fn>(name, attributes, data));
  return *this;
}

template <typename T>
template <auto getter>
inline ClassRegistration<T> &ClassRegistration<T>::StaticAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistrationEntry<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template StaticAccessor<getter>(name, attributes,
                                                          data));
  return *this;
}

template <typename T>
template <auto getter, auto setter>
inline ClassRegistration<T> &ClassRegistration<T>::StaticAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistrationEntry<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template StaticAccessor<getter, setter>(
          name, attributes, data));
  return *this;
}

}  // namespace NapiHelper

#define NAPI_HELPER_EXPORT                                          \
  static Napi::ModuleRegisterCallback NAPI_HELPER_MODULE_CALLBACK = \
      NapiHelper::Registration::ModuleCallback;                     \
  NODE_API_MODULE(NODE_GYP_MODULE_NAME, NAPI_HELPER_MODULE_CALLBACK)

#define NAPI_HELPER_REGISTRATION \
  NAPI_C_CTOR(napi_helper_auto_register_function_)

#endif  // SRC_NAPI_HELPER_INL_H_
