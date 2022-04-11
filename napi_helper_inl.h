#ifndef SRC_NAPI_HELPER_INL_H_
#define SRC_NAPI_HELPER_INL_H_

#include <optional>
#include <tuple>

#include <napi.h>

namespace NapiHelper {

namespace details {
template <typename T, typename Enabled = void> struct ValueTransformer {
  static std::optional<T> FromJS(Napi::Value);

  static Napi::Value ToJS(Napi::Env, const T &);
};

template <> struct ValueTransformer<int64_t> {
  static std::optional<int64_t> FromJS(Napi::Value value) {
    if (!value.IsBigInt()) {
      return {};
    }
    bool lossless = false;
    return value.As<Napi::BigInt>().Int64Value(&lossless);
  }

  static Napi::Value ToJS(Napi::Env env, const int64_t &num) {
    return Napi::BigInt::New(env, num);
  }
};

template <> struct ValueTransformer<uint64_t> {
  static std::optional<uint64_t> FromJS(Napi::Value value) {
    if (!value.IsBigInt()) {
      return {};
    }
    bool lossless = false;
    return value.As<Napi::BigInt>().Uint64Value(&lossless);
  }

  static Napi::Value ToJS(Napi::Env env, const uint64_t &num) {
    return Napi::BigInt::New(env, num);
  }
};

template <> struct ValueTransformer<double> {
  static std::optional<double> FromJS(Napi::Value value) {
    if (!value.IsBigInt()) {
      return {};
    }
    return value.As<Napi::Number>().DoubleValue();
  }

  static Napi::Value ToJS(Napi::Env env, const double &num) {
    return Napi::Number::New(env, num);
  }
};

template <> struct ValueTransformer<float> {
  static std::optional<float> FromJS(Napi::Value value) {
    if (!value.IsBigInt()) {
      return {};
    }
    return value.As<Napi::Number>().FloatValue();
  }

  static Napi::Value ToJS(Napi::Env env, const float &num) {
    return Napi::Number::New(env, num);
  }
};

template <typename Uint> // for uint8_t, uint16_t and uint32_t
struct ValueTransformer<
    Uint, typename std::enable_if_t<std::is_same_v<Uint, uint8_t> ||
                                    std::is_same_v<Uint, uint16_t> ||
                                    std::is_same_v<Uint, uint32_t>>> {
  static std::optional<Uint> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().Uint32Value();
  }

  static Napi::Value ToJS(Napi::Env env, const Uint &num) {
    return Napi::Number::New(env, num);
  }
};

template <typename Int> // for int8_t, int16_t and int32_t
struct ValueTransformer<
    Int, typename std::enable_if_t<std::is_same_v<Int, int8_t> ||
                                   std::is_same_v<Int, int16_t> ||
                                   std::is_same_v<Int, int32_t>>> {
  static std::optional<Int> FromJS(Napi::Value value) {
    if (!value.IsNumber()) {
      return {};
    }
    return value.As<Napi::Number>().Int32Value();
  }

  static Napi::Value ToJS(Napi::Env env, const Int &num) {
    return Napi::Number::New(env, num);
  }
};

template <> struct ValueTransformer<std::string> {
  static std::optional<std::string> FromJS(Napi::Value value) {
    if (!value.IsString()) {
      return {};
    }
    return value.As<Napi::String>().Utf8Value();
  }

  static Napi::Value ToJS(Napi::Env env, const std::string &str) {
    return Napi::String::New(env, str);
  }
};

template <> struct ValueTransformer<std::u16string> {
  static std::optional<std::u16string> FromJS(Napi::Value value) {
    if (!value.IsString()) {
      return {};
    }
    return value.As<Napi::String>().Utf16Value();
  }

  static Napi::Value ToJS(Napi::Env env, const std::u16string &str) {
    return Napi::String::New(env, str);
  }
};

template <typename T, typename Enable = void>
struct is_optional : std::false_type {};

template <typename T> struct is_optional<std::optional<T>> : std::true_type {};

template <class T> struct remove_optional { typedef T type; };
template <class T> struct remove_optional<std::optional<T>> { typedef T type; };

class Invoker {
private:
  template <size_t I, typename Head, typename... Rest>
  static std::optional<std::tuple<Head, Rest...>>
  CreateCallArgs(const Napi::CallbackInfo &info) {
    using Real = typename remove_optional<Head>::type;

    std::optional<Real> opt_head = ValueTransformer<Real>::FromJS(info[I]);

    if constexpr (!is_optional<Head>::value) {
      if (!opt_head.has_value()) {
        return {};
      }
    }

    std::optional<std::tuple<Rest...>> rest =
        CreateCallArgs<I + 1, Rest...>(info);
    if (!rest.has_value()) {
      return {};
    }

    if constexpr (is_optional<Head>::value) {
      return std::tuple_cat(std::make_tuple(std::move(opt_head)),
                            std::move(*rest));
    } else {
      return std::tuple_cat(std::make_tuple(std::move(opt_head.value())),
                            std::move(*rest));
    }
  }

  template <size_t I>
  static std::optional<std::tuple<>>
  CreateCallArgs(const Napi::CallbackInfo &) {
    return std::tuple<>();
  }

  template <typename Ret, typename... Args>
  static std::conditional_t<std::is_void_v<Ret>, void, Napi::Value>
  Call(const Napi::CallbackInfo &info, Ret (*fn)(Args...)) {
    std::optional<std::tuple<Args...>> args = CreateCallArgs<0, Args...>(info);

    if constexpr (std::is_void_v<Ret>) {
      if (!args.has_value()) {
        NAPI_THROW_VOID(Napi::TypeError::New(info.Env(), "bad arguments"));
      }
      std::apply(fn, std::move(args.value()));
    } else {
      if (!args.has_value()) {
        NAPI_THROW(Napi::TypeError::New(info.Env(), "bad arguments"),
                   Napi::Value());
      }
      Ret result = std::apply(fn, std::move(args.value()));
      return ValueTransformer<Ret>::ToJS(info.Env(), std::move(result));
    }
  }

  template <typename Ret, typename... Args>
  static std::conditional_t<std::is_void_v<Ret>, void, Napi::Value>
  Call(const Napi::CallbackInfo &info,
       Ret (*fn)(const Napi::CallbackInfo &, Args...)) {
    std::optional<std::tuple<Args...>> args = CreateCallArgs<0, Args...>(info);

    if constexpr (std::is_void_v<Ret>) {
      if (!args.has_value()) {
        NAPI_THROW_VOID(Napi::TypeError::New(info.Env(), "bad arguments"));
      }
      std::apply(fn, std::tuple_cat(std::make_tuple(std::cref((info)),
                                                    std::move(args.value()))));
    } else {
      if (!args.has_value()) {
        NAPI_THROW(Napi::TypeError::New(info.Env(), "bad arguments"),
                   Napi::Value());
      }
      Ret result =
          std::apply(fn, std::tuple_cat(std::make_tuple(std::cref(info)),
                                        std::move(args.value())));
      return ValueTransformer<Ret>::ToJS(info.Env(), std::move(result));
    }
  }

public:
  template <auto fn> static auto Callback(const Napi::CallbackInfo &info) {
    return Call(info, fn);
  }
};

} // namespace details

template <auto fn>
inline Napi::Function Function::New(Napi::Env env, const char *utf8name,
                                    void *data) {
  return Napi::Function::New<details::Invoker::Callback<fn>>(env, utf8name,
                                                             data);
}

} // namespace NapiHelper

#endif // SRC_NAPI_HELPER_H_