# Mix with node-addon-api

**naah** is based on node-addon-api by its name. All Napi values are supported as function arguments and return value.

## Function

`naah::Function` helps you to create `Napi::Function` with pure C++ function signature.

`naah::Function::New` behaves like `Napi::Function::New` and returns a `Napi::Function` instance.

```cpp
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
```

## ScriptWrappable

`naah::ScriptWrappable` is a replacement of `Napi::ObjectWrap` which provides a way to bind native class to JavaScript object.

```cpp
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

  using ConstructFn = std::unique_ptr<T> (*)(const Napi::CallbackInfo &);

  using PropertyDescriptor =
      typename Napi::ObjectWrap<This>::PropertyDescriptor;

  template <typename... Args>
  static std::unique_ptr<T> ConstructCallback(const Napi::CallbackInfo &);

  static Napi::Function DefineClass(
      Napi::Env env, const char *utf8name, ConstructFn ctor_fn,
      const std::initializer_list<PropertyDescriptor> &properties);

  static Napi::Function DefineClass(
      Napi::Env env, const char *utf8name, ConstructFn ctor_fn,
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

  template <auto getter, auto setter>
  static PropertyDescriptor StaticAccessor(
      Napi::Symbol name, napi_property_attributes attributes = napi_default,
      void *data = nullptr);

#ifdef NAAH_TAG_OBJECT_WRAP
  static const napi_type_tag *type_tag();
#endif
};
```

## Registration

`Registration` is the class to store values exported by `NAAH_REGISTRATION`.

```cpp
class Registration : public Napi::Addon<Registration> {
 public:
  template <typename T>
  static void Value(const char *name, T t);

  template <auto fn>
  static void Function(const char *name, void *data = nullptr);

  template <typename Callable>
  static void Function(const char *name, Callable callable,
                       void *data = nullptr);

  template <typename T, typename... CtorArgs>
  static ClassRegistration<T> Class(const char *name);

  template <typename T>
  static ObjectRegistration<T> Object();

  Registration(Napi::Env env, Napi::Object exports);

  template <typename T>
  Napi::Function FindClass();

 private:
  std::map<uint64_t, Napi::FunctionReference> classes_;
};
```
