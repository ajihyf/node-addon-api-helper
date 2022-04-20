#ifndef SRC_NAAH_INL_H_
#define SRC_NAAH_INL_H_

#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

namespace naah {

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
struct ValueTransformer<Napi::Uint8Array> {
  static std::optional<Napi::Uint8Array> FromJS(Napi::Value value) {
    if (!value.IsTypedArray()) {
      return {};
    }
    Napi::TypedArray arr = value.As<Napi::TypedArray>();
    if (arr.TypedArrayType() != napi_uint8_array &&
        arr.TypedArrayType() != napi_uint8_clamped_array) {
      return {};
    }
    return value.As<Napi::Uint8Array>();
  }

  static Napi::Value ToJS(Napi::Env, Napi::Uint8Array val) { return val; }
};

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

template <typename... Args>
struct ValueTransformer<std::tuple<Args...>> {
 private:
  using Tuple = std::tuple<Args...>;

  template <size_t... Is>
  static std::vector<napi_value> ToVectorImpl([[maybe_unused]] Napi::Env env,
                                              [[maybe_unused]] Tuple t,
                                              std::index_sequence<Is...>) {
    std::vector<napi_value> result(std::tuple_size_v<Tuple>);
    ((result[Is] = ValueTransformer<std::tuple_element_t<Is, Tuple>>::ToJS(
          env, std::move(std::get<Is>(t)))),
     ...);
    return result;
  }

 public:
  static std::vector<napi_value> ToVector(Napi::Env env, Tuple t) {
    return ToVectorImpl(env, std::move(t), std::index_sequence_for<Args...>{});
  }

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

          std::vector<napi_value> vec = ToVector(env, std::move(t));
          for (uint32_t i = 0; i < vec.size(); i++) {
            result.Set(i, Napi::Value(env, vec[i]));
          }

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

template <>
struct ValueTransformer<ArrayBuffer> {
  static std::optional<ArrayBuffer> FromJS(Napi::Value value) {
    if (!value.IsArrayBuffer()) {
      return {};
    }
    Napi::ArrayBuffer buf = value.As<Napi::ArrayBuffer>();
    return ArrayBuffer(static_cast<char *>(buf.Data()),
                       static_cast<char *>(buf.Data()) + buf.ByteLength());
  }

  static Napi::Value ToJS(Napi::Env env, ArrayBuffer arr) {
    ArrayBuffer *arr_ptr = new ArrayBuffer(std::move(arr));
    return Napi::ArrayBuffer::New(
        env, arr_ptr->data(), arr_ptr->size(),
        [](napi_env, void *, void *hint) {
          delete static_cast<ArrayBuffer *>(hint);
        },
        arr_ptr);
  }
};

template <typename E, napi_typedarray_type type>
struct ValueTransformer<TypedArrayOf<E, type>> {
 private:
  using T = TypedArrayOf<E, type>;

 public:
  static std::optional<T> FromJS(Napi::Value value) {
    if (!value.IsTypedArray()) {
      return {};
    }
    Napi::TypedArray arr = value.As<Napi::TypedArray>();
    if (arr.TypedArrayType() != type) {
      return {};
    }
    Napi::TypedArrayOf<E> typed_arr = arr.As<Napi::TypedArrayOf<E>>();
    return T(typed_arr->Data(), typed_arr->Data() + typed_arr->Length());
  }

  static Napi::Value ToJS(Napi::Env env, T arr) {
    T *arr_ptr = new T(std::move(arr));
    Napi::ArrayBuffer buf = Napi::ArrayBuffer::New(
        env, arr_ptr->data(), arr_ptr->size() * sizeof(T),
        [](napi_env, void *, void *hint) { delete static_cast<T *>(hint); },
        arr_ptr);
    return Napi::TypedArrayOf<E>::New(env, arr_ptr->size(), buf, 0, type);
  }
};

#ifdef NAAH_TAG_OBJECT_WRAP

template <typename T>
struct ValueTransformer<T *, std::enable_if_t<std::is_base_of_v<Class, T>>> {
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

template <typename T>
struct ValueTransformer<T, std::enable_if_t<std::is_base_of_v<Class, T>>> {
 public:
  static Napi::Value ToJS(Napi::Env env, T t) {
    Registration *reg = env.GetInstanceData<Registration>();
    Napi::Function clazz = reg->FindClass<T>();
    if (clazz.IsEmpty()) {
      return env.Undefined();  // prevent crash
    }

    // RAII, maybe moved in ScriptWrappable constructor
    std::unique_ptr<T> p(new T(std::move(t)));

    return clazz.New(
        {Napi::External<napi_type_tag>::New(
             env, const_cast<napi_type_tag *>(ScriptWrappable<T>::type_tag())),
         Napi::External<std::unique_ptr<T>>::New(env, &p)});
  }
};

#endif

template <typename E>
struct ValueTransformer<E,
                        std::enable_if_t<std::is_base_of_v<naah::Error, E>>> {
 public:
  static Napi::Value ToJS(Napi::Env env, E e) {
    return E::JSError::New(env, e.Message()).Value();
  }
};

namespace details {

template <typename T>
class TSFNContainer;

template <typename... Args>
class TSFNContainer<std::function<void(Args...)>> {
 public:
  NAPI_DISALLOW_ASSIGN_COPY(TSFNContainer)

  TSFNContainer(Napi::Function fun)
      : _tsfn(TSFN::New(fun.Env(), fun, "naah::details::TSFNContainer", 0, 1)) {
  }

  ~TSFNContainer() { _tsfn.Release(); }

  static std::function<void(Args...)> Create(Napi::Function fun) {
    return [tsfn = std::make_shared<TSFNContainer>(fun)](Args... args) {
      tsfn->Call(std::move(args)...);
    };
  }

 private:
  using Tuple = std::tuple<Args...>;

  void Call(Args &&...args) {
    std::unique_ptr<Tuple> t =
        std::make_unique<Tuple>(std::forward<Args>(args)...);
    if (_tsfn.NonBlockingCall(t.get()) == napi_ok) {
      t.release();
    }
  }

  static void CallJS(Napi::Env env, Napi::Function fun, std::nullptr_t *,
                     Tuple *_data) {
    std::unique_ptr<Tuple> t(_data);  // RAII
    std::vector<napi_value> args =
        ValueTransformer<Tuple>::ToVector(env, std::move(*t));

#ifdef NAPI_CPP_EXCEPTIONS
    try {
      fun.Call(args);
    } catch (const Napi::Error &e) {
      napi_fatal_exception(env, e.Value());
    }
#else
    Napi::Value result = fun.Call(args);
    if (result.IsEmpty()) {
      Napi::Error e = env.GetAndClearPendingException();
      napi_fatal_exception(env, e.Value());
    }
#endif
  }

  using TSFN = Napi::TypedThreadSafeFunction<std::nullptr_t, Tuple, CallJS>;

  TSFN _tsfn;
};

template <typename T, typename Enable = void>
struct is_std_function : std::false_type {};

template <typename Ret, typename... Args>
struct is_std_function<std::function<Ret(Args...)>> : std::true_type {};

}  // namespace details

template <typename Callable>
struct ValueTransformer<
    Callable,
    std::enable_if_t<
        std::is_function_v<std::remove_pointer_t<Callable>> ||
        std::is_member_function_pointer_v<decltype(&Callable::operator())>>> {
  static std::optional<Callable> FromJS(Napi::Value value) {
    static_assert(details::is_std_function<Callable>::value,
                  "arguments fn can only be std::function<void(Args...)>");
    if (!value.IsFunction()) {
      return {};
    }
    Napi::Function fun = value.As<Napi::Function>();
    return details::TSFNContainer<Callable>::Create(fun);
  }

  static Napi::Value ToJS(Napi::Env env, Callable v) {
    return Function::New(env, std::move(v));
  }
};

template <typename Ret>
template <typename Arg>
inline AsyncWork<Ret>::AsyncWork(Arg &&arg) : task(std::forward<Arg>(arg)) {}

template <typename T, typename E>
inline Result<T, E>::Result(T t) : value(std::move(t)) {}

template <typename T, typename E>
inline Result<T, E>::Result(E e) : error(std::move(e)) {}

namespace details {

class AsyncWorkWorker : public Napi::AsyncWorker {
 public:
  AsyncWorkWorker(Napi::Env env, AsyncWork<void> task)
      : Napi::AsyncWorker(env), _task(std::move(task)) {}

  ~AsyncWorkWorker() {}

  void Execute() override { _task.task(); }

 private:
  AsyncWork<void> _task;
};

template <typename T, typename Enable = void>
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<Result<T, E>> : std::true_type {};

template <class T>
struct result_type;

template <typename T_, typename E_>
struct result_type<Result<T_, E_>> {
  typedef T_ T;
  typedef E_ E;
};

template <typename T>
class PromiseAsyncWorker : public Napi::AsyncWorker {
 public:
  PromiseAsyncWorker(Napi::Env env, AsyncWork<T> task)
      : Napi::AsyncWorker(env), _task(std::move(task)), _deferred(env) {}

  ~PromiseAsyncWorker() {}

  Napi::Promise Promise() { return _deferred.Promise(); }

  void Execute() override { _result = _task.task(); }

  void OnOK() override {
    if (_result.has_value()) {
      if constexpr (is_result<T>::value) {
        using Resolve = typename result_type<T>::T;
        using Reject = typename result_type<T>::E;

        auto &promise_result = _result.value();
        if (promise_result.value.has_value()) {
          _deferred.Resolve(ValueTransformer<Resolve>::ToJS(
              Env(), std::move(*promise_result.value)));
        } else {
          if constexpr (!std::is_same_v<Reject, std::nullptr_t>) {
            if (_result.value().error.has_value()) {
              _deferred.Reject(ValueTransformer<Reject>::ToJS(
                  Env(), std::move(*promise_result.error)));
            }
          }
        }
      } else {
        _deferred.Resolve(
            ValueTransformer<T>::ToJS(Env(), std::move(*_result)));
      }
    }
  }

 private:
  AsyncWork<T> _task;
  std::optional<T> _result;
  Napi::Promise::Deferred _deferred;
};
}  // namespace details

template <>
struct ValueTransformer<AsyncWork<void>> {
  static Napi::Value ToJS(Napi::Env env, AsyncWork<void> task) {
    auto *task_worker = new details::AsyncWorkWorker(env, std::move(task));
    task_worker->Queue();
    return env.Undefined();
  }
};

template <typename T>
struct ValueTransformer<AsyncWork<T>, std::enable_if_t<!std::is_void_v<T>>> {
  static Napi::Value ToJS(Napi::Env env, AsyncWork<T> task) {
    auto *promise_worker =
        new details::PromiseAsyncWorker<T>(env, std::move(task));
    promise_worker->Queue();
    return promise_worker->Promise();
  }
};

template <typename T>
inline Napi::Value ConvertToJS(Napi::Env env, T v) {
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
      if constexpr (details::is_result<Ret>::value) {
        using T = typename result_type<Ret>::T;
        using E = typename result_type<Ret>::E;
        if (result.value.has_value()) {
          return ValueTransformer<T>::ToJS(info.Env(),
                                           std::move(*result.value));
        } else {
          E::JSError::New(info.Env(), (*result.error).Message())
              .ThrowAsJavaScriptException();
          return info.Env().Undefined();
        }
      } else {
        return ValueTransformer<Ret>::ToJS(info.Env(), std::move(result));
      }
    }
  }

  template <auto fn>
  static auto FunctionCallback(const Napi::CallbackInfo &info) {
    return CallJS(info, fn);
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
#ifdef NAAH_TAG_OBJECT_WRAP
  if constexpr (std::is_base_of_v<Class, T>) {
    napi_type_tag_object(info.Env(), info.This(), type_tag());
    if (info[0].IsExternal() &&
        info[0].As<Napi::External<napi_type_tag>>().Data() == type_tag()) {
      // created by ValueTransformer
      _wrapped =
          std::move(*info[1].As<Napi::External<std::unique_ptr<T>>>().Data());
      return;
    }
  }
#endif

  auto t_ctor = reinterpret_cast<ConstructFn>(info.Data());

  if (t_ctor == nullptr) {
    NAPI_THROW_VOID(Napi::Error::New(info.Env(), "not constructible by new"));
  }

  _wrapped = t_ctor(info);
}

template <typename T>
inline T &ScriptWrappable<T>::wrapped() const {
  return *_wrapped;
}

template <typename T>
inline Napi::Function ScriptWrappable<T>::DefineClass(
    Napi::Env env, const char *utf8name, ConstructFn fn,
    const std::initializer_list<PropertyDescriptor> &properties) {
  using Wrapped = Napi::ObjectWrap<This>;

  return Wrapped::DefineClass(env, utf8name, properties,
                              reinterpret_cast<void *>(fn));
}

template <typename T>
inline Napi::Function ScriptWrappable<T>::DefineClass(
    Napi::Env env, const char *utf8name, ConstructFn fn,
    const std::vector<PropertyDescriptor> &properties) {
  using Wrapped = Napi::ObjectWrap<This>;

  return Wrapped::DefineClass(env, utf8name, properties,
                              reinterpret_cast<void *>(fn));
}

template <typename T>
template <typename... Args>
inline std::unique_ptr<T> ScriptWrappable<T>::ConstructCallback(
    const Napi::CallbackInfo &info) {
  return details::Invoker::Call(info, [](Args... args) -> std::unique_ptr<T> {
    return std::unique_ptr<T>(new T(std::move(args)...));
  });
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

#ifdef NAAH_TAG_OBJECT_WRAP

template <typename T>
inline const napi_type_tag *ScriptWrappable<T>::type_tag() {
  static_assert(std::is_base_of_v<Class, T>, "T must inherits Class");
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

struct ClassRegistrationEntry {
  const char *name;
  uint64_t type_tag;
  std::function<Napi::Function(Napi::Env, const char *)> init_cb;

  static std::vector<ClassRegistrationEntry> &Entries() {
    static std::vector<ClassRegistrationEntry> entries;
    return entries;
  }
};

template <typename T>
class ClassRegistration {
 private:
  const char *_name = nullptr;
  typename ScriptWrappable<T>::ConstructFn _ctor_fn = nullptr;
  std::vector<typename ScriptWrappable<T>::PropertyDescriptor> _descriptors;

  static ClassRegistration &Instance() {
    static ClassRegistration instance;
    return instance;
  }

  static Napi::Function DefineClassCallback(Napi::Env env, const char *name) {
    ClassRegistration &instance = Instance();

    return ScriptWrappable<T>::DefineClass(env, name, instance._ctor_fn,
                                           instance._descriptors);
  }

 public:
  static void DefineClass() {
    ClassRegistration &instance = Instance();
    ClassRegistrationEntry::Entries().push_back(ClassRegistrationEntry{
        instance._name, ScriptWrappable<T>::type_tag()->lower,
        DefineClassCallback});
  }

  static void AddPropertyDescriptor(
      typename ScriptWrappable<T>::PropertyDescriptor descriptor) {
    Instance()._descriptors.push_back(std::move(descriptor));
  }

  static void SetName(const char *name) { Instance()._name = name; }

  static void SetConstructor(typename ScriptWrappable<T>::ConstructFn fn) {
    Instance()._ctor_fn = fn;
  }
};
}  // namespace details

inline Registration::Registration(Napi::Env env, Napi::Object exports) {
  for (auto &it : details::RegistrationEntry::Entries()) {
    exports.Set(it.name, it.init_cb(env, it.name));
  }
  for (auto &it : details::ClassRegistrationEntry::Entries()) {
    Napi::Function fun = it.init_cb(env, it.name);
    classes_[it.type_tag] = Napi::Persistent(fun);
    exports.Set(it.name, fun);
  }
}

template <typename T>
inline Napi::Function Registration::FindClass() {
  uint64_t key = ScriptWrappable<T>::type_tag()->lower;
  auto it = classes_.find(key);
  return it == classes_.end() ? Napi::Function() : it->second.Value();
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

template <typename T>
inline ClassRegistration<T> Registration::Class(const char *name) {
  details::ClassRegistration<T>::SetName(name);
  return ClassRegistration<T>();
}

template <typename T>
inline ClassRegistration<T>::~ClassRegistration() {
  details::ClassRegistration<T>::DefineClass();
}

template <typename T>
template <typename... CtorArgs>
inline ClassRegistration<T> &ClassRegistration<T>::Constructor() {
  details::ClassRegistration<T>::SetConstructor(
      ScriptWrappable<T>::template ConstructCallback<CtorArgs...>);

  return *this;
}

template <typename T>
template <auto T::*fn>
inline ClassRegistration<T> &ClassRegistration<T>::InstanceMethod(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistration<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template InstanceMethod<fn>(name, attributes, data));
  return *this;
}

template <typename T>
template <auto T::*getter>
inline ClassRegistration<T> &ClassRegistration<T>::InstanceAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistration<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template InstanceAccessor<getter>(name, attributes,
                                                            data));
  return *this;
}

template <typename T>
template <auto T::*getter, auto T::*setter>
inline ClassRegistration<T> &ClassRegistration<T>::InstanceAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistration<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template InstanceAccessor<getter, setter>(
          name, attributes, data));
  return *this;
}

template <typename T>
template <auto fn>
inline ClassRegistration<T> &ClassRegistration<T>::StaticMethod(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistration<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template StaticMethod<fn>(name, attributes, data));
  return *this;
}

template <typename T>
template <auto getter>
inline ClassRegistration<T> &ClassRegistration<T>::StaticAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistration<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template StaticAccessor<getter>(name, attributes,
                                                          data));
  return *this;
}

template <typename T>
template <auto getter, auto setter>
inline ClassRegistration<T> &ClassRegistration<T>::StaticAccessor(
    const char *name, napi_property_attributes attributes, void *data) {
  details::ClassRegistration<T>::AddPropertyDescriptor(
      ScriptWrappable<T>::template StaticAccessor<getter, setter>(
          name, attributes, data));
  return *this;
}

namespace details {

template <typename T>
struct ObjectFieldEntry {
  std::function<bool(Napi::Object, T &)> FromJS;
  std::function<void(T &, Napi::Object)> ToJS;
};

template <typename T>
class ObjectFieldEntryStore {
 public:
  static std::vector<ObjectFieldEntry<T>> &descriptors() {
    static std::vector<ObjectFieldEntry<T>> descriptors_;
    return descriptors_;
  }

  template <typename M>
  static void AddField(const char *name, M T::*m) {
    using Real = typename remove_optional<M>::type;

    descriptors().push_back(
        {[name, m](Napi::Object js_obj, T &obj) -> bool {
           std::optional<Real> v =
               ValueTransformer<Real>::FromJS(js_obj.Get(name));
           if constexpr (!is_optional<M>::value) {
             if (!v.has_value()) {
               return false;
             }
             obj.*m = std::move(*v);
           } else {
             obj.*m = std::move(v);
           }

           return true;
         },
         [name, m](T &obj, Napi::Object js_obj) -> void {
           if constexpr (is_optional<M>::value) {
             if (!((obj.*m).has_value())) {
               return;
             }
             js_obj.Set(name, ValueTransformer<Real>::ToJS(
                                  js_obj.Env(), std::move(*(obj.*m))));
           } else {
             js_obj.Set(name, ValueTransformer<Real>::ToJS(js_obj.Env(),
                                                           std::move(obj.*m)));
           }
         }});
  }
};

}  // namespace details

template <typename T>
struct ValueTransformer<T, std::enable_if_t<std::is_base_of_v<Object, T>>> {
  static std::optional<T> FromJS(Napi::Value value) {
    if (!value.IsObject()) {
      return {};
    }
    Napi::Object obj = value.As<Napi::Object>();
    T t;
    for (auto &it : details::ObjectFieldEntryStore<T>::descriptors()) {
      if (!it.FromJS(obj, t)) {
        return {};
      }
    }
    return std::move(t);
  }

  static Napi::Value ToJS(Napi::Env env, T v) {
    Napi::Object obj = Napi::Object::New(env);
    for (auto &it : details::ObjectFieldEntryStore<T>::descriptors()) {
      it.ToJS(v, obj);
    }
    return obj;
  }
};

template <typename T>
template <auto T::*m>
inline ObjectRegistration<T> ObjectRegistration<T>::Member(const char *name) {
  details::ObjectFieldEntryStore<T>::AddField(name, m);
  return *this;
}

template <typename T>
inline ObjectRegistration<T> Registration::Object() {
  return ObjectRegistration<T>();
}

}  // namespace naah

#define NAAH_EXPORT                                   \
  using NAAH_REGISTRATION_ADDON = naah::Registration; \
  NODE_API_ADDON(NAAH_REGISTRATION_ADDON)

#define NAAH_REGISTRATION NAPI_C_CTOR(napi_helper_auto_register_function_)

#endif  // SRC_NAAH_INL_H_
