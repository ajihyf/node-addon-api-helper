#ifndef SRC_NAPI_HELPER_INL_H_
#define SRC_NAPI_HELPER_INL_H_

#include <optional>
#include <string>
#include <tuple>
#include <variant>

namespace NapiHelper {

namespace details {
template <typename T, typename Enabled = void>
struct ValueTransformer {
  static std::optional<T> FromJS(Napi::Value);

  static Napi::Value ToJS(Napi::Env, T &);
};

template <>
struct ValueTransformer<Napi::Value> {
  static std::optional<Napi::Value> FromJS(Napi::Value v) { return v; }
  static Napi::Value ToJS(Napi::Env, Napi::Value &val) { return val; }
};

template <typename T, bool (Napi::Value::*IsT)() const>
struct JSValueTransformer {
  static std::optional<T> FromJS(Napi::Value value) {
    if (!(value.*IsT)()) {
      return {};
    }
    return value.As<T>();
  }

  static Napi::Value ToJS(Napi::Env, T &val) { return val; }
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
struct ValueTransformer<Napi::Date> : public JSValueTransformer<Napi::Date, &Napi::Value::IsDate> {
};
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
    : public JSValueTransformer<Napi::ArrayBuffer, &Napi::Value::IsArrayBuffer> {};

template <>
struct ValueTransformer<Napi::TypedArray>
    : public JSValueTransformer<Napi::TypedArray, &Napi::Value::IsTypedArray> {};

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

  static Napi::Value ToJS(Napi::Env, T &val) { return val; }
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
    : public TypedArrayTransformer<Napi::BigUint64Array, napi_biguint64_array> {};
#endif

template <>
struct ValueTransformer<bool> {
  static std::optional<bool> FromJS(Napi::Value value) {
    if (!value.IsBoolean()) {
      return {};
    }
    return value.As<Napi::Boolean>().Value();
  }

  static Napi::Value ToJS(Napi::Env env, bool &num) { return Napi::Boolean::New(env, num); }
};

template <>
struct ValueTransformer<double> {
  static std::optional<double> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().DoubleValue();
  }

  static Napi::Value ToJS(Napi::Env env, double &num) { return Napi::Number::New(env, num); }
};

template <>
struct ValueTransformer<float> {
  static std::optional<float> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().FloatValue();
  }

  static Napi::Value ToJS(Napi::Env env, float &num) { return Napi::Number::New(env, num); }
};

template <typename Uint>
struct ValueTransformer<Uint, typename std::enable_if_t<std::is_same_v<Uint, uint8_t> ||
                                                        std::is_same_v<Uint, uint16_t> ||
                                                        std::is_same_v<Uint, uint32_t>>> {
  static std::optional<Uint> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().Uint32Value();
  }

  static Napi::Value ToJS(Napi::Env env, Uint &num) { return Napi::Number::New(env, num); }
};

template <typename Int>
struct ValueTransformer<
    Int, typename std::enable_if_t<std::is_same_v<Int, int8_t> || std::is_same_v<Int, int16_t> ||
                                   std::is_same_v<Int, int32_t>>> {
  static std::optional<Int> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().Int32Value();
  }

  static Napi::Value ToJS(Napi::Env env, Int &num) { return Napi::Number::New(env, num); }
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

  static Napi::Value ToJS(Napi::Env env, int64_t &num) { return Napi::BigInt::New(env, num); }
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

  static Napi::Value ToJS(Napi::Env env, uint64_t &num) { return Napi::BigInt::New(env, num); }
};
#endif

template <>
struct ValueTransformer<std::string> {
  static std::optional<std::string> FromJS(Napi::Value value) {
    if (!value.IsString()) {
      return {};
    }
    return value.As<Napi::String>().Utf8Value();
  }

  static Napi::Value ToJS(Napi::Env env, std::string &str) { return Napi::String::New(env, str); }
};

template <>
struct ValueTransformer<std::u16string> {
  static std::optional<std::u16string> FromJS(Napi::Value value) {
    if (!value.IsString()) {
      return {};
    }
    return value.As<Napi::String>().Utf16Value();
  }

  static Napi::Value ToJS(Napi::Env env, std::u16string &str) {
    return Napi::String::New(env, str);
  }
};

template <typename T>
struct ValueTransformer<std::optional<T>> {
  static Napi::Value ToJS(Napi::Env env, std::optional<T> &val) {
    if (val.has_value()) {
      return ValueTransformer<T>::ToJS(env, *val);
    }
    return Napi::Value();
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
  static std::optional<Variants> FromJS(Napi::Value value) { return FromJS<0>(value); }

  static Napi::Value ToJS(Napi::Env env, Variants &v) {
    return std::visit(
        [&](auto &&arg) -> Napi::Value {
          using T = std::decay_t<decltype(arg)>;
          return ValueTransformer<T>::ToJS(env, arg);
        },
        v);
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

  static Napi::Value ToJS(Napi::Env env, std::vector<T> &arr) {
    Napi::Array result = Napi::Array::New(env, arr.size());
    for (uint32_t i = 0; i < arr.size(); i++) {
      result.Set(i, ValueTransformer<T>::ToJS(env, arr[i]));
    }
    return result;
  }
};

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
        ArgsConverter<std::tuple<Rest...>>::Get(std::forward<Args>(args), i + 1);
    if (!rest.has_value()) {
      return {};
    }

    if constexpr (is_optional<Head>::value) {
      return std::tuple_cat(std::make_tuple(std::move(opt_head)), std::move(*rest));
    } else {
      return std::tuple_cat(std::make_tuple(std::move(*opt_head)), std::move(*rest));
    }
  }
};

template <typename... Args>
struct ValueTransformer<std::tuple<Args...>> {
 private:
  using Tuple = std::tuple<Args...>;

  template <size_t... Is>
  static void SetArrayElement(Napi::Env env, Napi::Array &arr, Tuple &t,
                              std::index_sequence<Is...>) {
    (arr.Set(static_cast<uint32_t>(Is),
             ValueTransformer<std::tuple_element_t<Is, Tuple>>::ToJS(env, std::get<Is>(t))),
     ...);
  }

 public:
  static std::optional<Tuple> FromJS(Napi::Value value) {
    if (!value.IsArray()) {
      return {};
    }
    return ArgsConverter<Tuple>::Get(value.As<Napi::Array>(), 0);
  }

  static Napi::Value ToJS(Napi::Env env, Tuple &t) {
    return std::apply(
        [&](auto &&...elements) -> Napi::Array {
          constexpr size_t size = sizeof...(elements);

          Napi::Array result = Napi::Array::New(env, size);

          SetArrayElement(env, result, t, std::make_index_sequence<size>());

          return result;
        },
        t);
  }
};

class Invoker {
 private:
  template <typename Callable>
  static auto CallInternal(const Napi::CallbackInfo &info, Callable &&fn,
                           const char *bad_argument_message) {
    using Signature = get_signature<std::remove_cv_t<std::remove_reference_t<Callable>>>;
    using Ret = typename Signature::ret;
    using OriginArgs = typename Signature::args;

    constexpr bool head_is_cb_info =
        std::tuple_size_v<OriginArgs> != 0 &&
        std::is_same_v<typename get_tuple_elements<OriginArgs>::head, const Napi::CallbackInfo &>;

    using Args =
        typename std::conditional_t<head_is_cb_info, typename get_tuple_elements<OriginArgs>::rest,
                                    OriginArgs>;

    std::optional<Args> args = ArgsConverter<Args>::Get(info, 0);
    if (bad_argument_message == nullptr) {
      bad_argument_message = "bad arguments";
    }

    if constexpr (std::is_void_v<Ret>) {
      if (!args.has_value()) {
        NAPI_THROW_VOID(Napi::TypeError::New(info.Env(), bad_argument_message));
      }
      if constexpr (head_is_cb_info) {
        std::apply(std::forward<Callable>(fn),
                   std::tuple_cat(std::make_tuple(std::cref(info)), std::move(*args)));
      } else {
        std::apply(std::forward<Callable>(fn), std::move(*args));
      }
    } else {
      if (!args.has_value()) {
        NAPI_THROW(Napi::TypeError::New(info.Env(), bad_argument_message), Napi::Value());
      }
      if constexpr (head_is_cb_info) {
        Ret result = std::apply(std::forward<Callable>(fn),
                                std::tuple_cat(std::make_tuple(std::cref(info)), std::move(*args)));
        return ValueTransformer<Ret>::ToJS(info.Env(), result);
      } else {
        Ret result = std::apply(std::forward<Callable>(fn), std::move(*args));
        return ValueTransformer<Ret>::ToJS(info.Env(), result);
      }
    }
  }

 public:
  template <typename Callable>
  static auto Call(const Napi::CallbackInfo &info, Callable &&fn,
                   const char *bad_arguments_message) {
#ifdef NAPI_CPP_EXCEPTIONS
    try {
      return CallInternal(info, std::forward<Callable>(fn), bad_arguments_message);
    } catch (const RangeError &err) {
      throw Napi::RangeError::New(info.Env(), err.Message());
    } catch (const TypeError &err) {
      throw Napi::TypeError::New(info.Env(), err.Message());
    } catch (const Error &err) {
      throw Napi::Error::New(info.Env(), err.Message());
    }
#else
    return CallInternal(info, std::forward<Callable>(fn));
#endif
  }

  template <auto fn, const char *bad_arguments_message>
  static auto Callback(const Napi::CallbackInfo &info) {
    return Call(info, fn, bad_arguments_message);
  }
};

}  // namespace details

inline Error::Error(const char *msg) : _message(msg) {}
inline Error::Error(const std::string &msg) : _message(msg) {}

inline const std::string &Error::Message() const NAPI_NOEXCEPT { return _message; }

#ifdef NAPI_CPP_EXCEPTIONS

inline const char *Error::what() const NAPI_NOEXCEPT { return _message.c_str(); }

#endif  // NAPI_CPP_EXCEPTIONS

template <auto fn, const char *bad_arguments_message>
inline Napi::Function Function::New(Napi::Env env, const char *utf8name, void *data) {
  return Napi::Function::New<details::Invoker::Callback<fn, bad_arguments_message>>(env, utf8name,
                                                                                    data);
}

template <typename Callable>
inline Napi::Function Function::New(Napi::Env env, Callable fn, const char *utf8name, void *data,
                                    const char *bad_arguments_message) {
  return Napi::Function::New(
      env, [ fn = std::move(fn), bad_arguments_message ](const Napi::CallbackInfo &info) -> auto {
        return details::Invoker::Call(info, fn, bad_arguments_message);
      },
      utf8name, data);
}
}  // namespace NapiHelper

#endif  // SRC_NAPI_HELPER_INL_H_
