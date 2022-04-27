# Mix with node-addon-api

## Use Napi value

**naah** is based on node-addon-api by its name. All Napi values are supported as function arguments and return value.

```cpp
void setObjectKey(Napi::Object obj, std::string key, std::string v) {
  obj[key] = Napi::String::New(obj.Env(), v);
}
```

## Register exports manually

If you don't want to use `NAAH_EXPORTS` to self register a native module, you can init values exported by `NAAH_REGISTRATION` manually :

```cpp
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // existing exports

  naah::Registration::Init(env, exports);

  return exports;
}

NODE_API_MODULE(addon, Init)
```
