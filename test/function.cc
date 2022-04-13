#include <napi_helper.h>

namespace {
uint32_t ArgsCallback(int32_t num1, std::optional<uint32_t> num2,
                      std::string str, bool b) {
  return num1 + num2.value_or(42) + str.size() + b;
}

void VoidCallback(int32_t) {}

uint32_t ValueCallbackWithInfo(const Napi::CallbackInfo &info, int32_t num1) {
  return num1 + info.Length();
}

NapiHelper::Undefined UndefinedCallback(NapiHelper::Undefined) {
  return NapiHelper::Undefined{};
}

NapiHelper::Null NullCallback(NapiHelper::Null) { return NapiHelper::Null{}; }

Napi::Value ValueCallback(Napi::Value value) { return value.ToString(); }

Napi::Boolean BooleanCallback(Napi::Boolean value) {
  return Napi::Boolean::New(value.Env(), !value.Value());
}

Napi::Number NumberCallback(Napi::Number value) {
  return Napi::Number::New(value.Env(), value.Uint32Value() + 1);
}

Napi::Symbol SymbolCallback(Napi::Symbol symbol) { return symbol; }

Napi::BigInt BigIntCallback(Napi::BigInt value) {
  bool lossless = false;
  return Napi::BigInt::New(value.Env(), value.Int64Value(&lossless) + 233);
}

Napi::String StringCallback(Napi::String str) {
  return Napi::String::New(str.Env(), str.Utf8Value() + " world");
}

Napi::Date DateCallback(Napi::Date date) {
  return Napi::Date::New(date.Env(), date.ValueOf() + 233);
}

Napi::Object ObjectCallback(Napi::Object obj) {
  obj.Set("key", "value");
  return obj;
}

Napi::Array ArrayCallback(Napi::Array arr) {
  arr.Set(static_cast<uint32_t>(0), Napi::Number::New(arr.Env(), 233));
  return arr;
}

Napi::ArrayBuffer ArrayBufferCallback(Napi::ArrayBuffer arr) {
  *static_cast<uint8_t *>(arr.Data()) = 233;
  return arr;
}

Napi::Function FunctionCallback(Napi::Function fun) { return fun; }

Napi::Promise PromiseCallback(Napi::Promise p) { return p; }

Napi::DataView DataViewCallback(Napi::DataView d) { return d; }

Napi::TypedArray TypedArrayCallback(Napi::TypedArray arr) { return arr; }

Napi::External<void *> ExternalCallback(Napi::External<void *> ext) {
  return ext;
}

Napi::Buffer<uint8_t> BufferCallback(Napi::Buffer<uint8_t> buf) {
  buf[0] = 233;
  return buf;
}

Napi::Int8Array Int8ArrayCallback(Napi::Int8Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::Uint8Array Uint8ArrayCallback(Napi::Uint8Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::Int16Array Int16ArrayCallback(Napi::Int16Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::Uint16Array Uint16ArrayCallback(Napi::Uint16Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::Int32Array Int32ArrayCallback(Napi::Int32Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::Uint32Array Uint32ArrayCallback(Napi::Uint32Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::Float32Array Float32ArrayCallback(Napi::Float32Array arr) {
  arr[0] = 42.233;
  return arr;
}

Napi::Float64Array Float64ArrayCallback(Napi::Float64Array arr) {
  arr[0] = 42.233;
  return arr;
}

Napi::BigInt64Array BigInt64ArrayCallback(Napi::BigInt64Array arr) {
  arr[0] = 42;
  return arr;
}

Napi::BigUint64Array BigUint64ArrayCallback(Napi::BigUint64Array arr) {
  arr[0] = 42;
  return arr;
}

bool BoolCallback(bool b) { return !b; }

double DoubleCallback(double num) { return num + 1; }

float FloatCallback(float num) { return num + 1; }

uint8_t Uint8Callback(uint8_t num) { return num + 1; }

uint16_t Uint16Callback(uint16_t num) { return num + 1; }

uint32_t Uint32Callback(uint32_t num) { return num + 1; }

int8_t Int8Callback(int8_t num) { return num + 1; }

int16_t Int16Callback(int16_t num) { return num + 1; }

int32_t Int32Callback(int32_t num) { return num + 1; }

int64_t Int64Callback(int64_t num) { return num + 42; }

uint64_t Uint64Callback(uint64_t num) { return num + 233; }

std::string StrCallback(std::string str) { return str + "!!"; }

std::u16string U16StrCallback(std::u16string str) { return str + u"??"; }

std::vector<uint32_t> VectorCallback(std::vector<uint32_t> arr) {
  for (auto &num : arr) {
    num++;
  }
  return arr;
}

std::tuple<uint32_t, std::optional<std::string>> TupleCallback(
    std::tuple<std::string, std::optional<uint32_t>> input) {
  std::optional<std::string> ret1;
  if (auto &arg1 = std::get<1>(input)) {
    ret1.emplace(std::to_string(*arg1));
  }
  return std::make_tuple(std::get<0>(input).size(), std::move(ret1));
}

std::variant<uint32_t, std::string> FunctionWithVariants(
    std::variant<uint32_t, std::string> input) {
  struct Visitor {
    std::variant<uint32_t, std::string> operator()(std::string &str) {
      return str.size();
    }
    std::variant<uint32_t, std::string> operator()(uint32_t &num) {
      return std::to_string(num);
    }
  };

  return std::visit(Visitor(), input);
}

std::string FunctionThrows(uint32_t i) {
  if (i > 42) {
    throw NapiHelper::RangeError("bigger than 42");
  }
  return std::to_string(i);
}

std::optional<std::string> FunctionThrowsManually(
    const Napi::CallbackInfo &info, uint32_t i) {
  if (i > 233) {
    Napi::RangeError::New(info.Env(), "bigger than 233")
        .ThrowAsJavaScriptException();
    return {};
  }
  return std::to_string(i);
}
}  // namespace

Napi::Object InitFunction(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  obj["argsCallback"] = NapiHelper::Function::New<ArgsCallback>(env);

  obj["customBadArgumentsCallbackTpl"] =
      NapiHelper::Function::New<ArgsCallback>(env);
  obj["customBadArgumentsCallback"] =
      NapiHelper::Function::New(env, ArgsCallback, nullptr, nullptr);

  obj["undefinedCallback"] = NapiHelper::Function::New<UndefinedCallback>(env);
  obj["nullCallback"] = NapiHelper::Function::New<NullCallback>(env);
  obj["valueCallback"] = NapiHelper::Function::New<ValueCallback>(env);
  obj["booleanCallback"] = NapiHelper::Function::New<BooleanCallback>(env);
  obj["numberCallback"] = NapiHelper::Function::New<NumberCallback>(env);
  obj["bigIntCallback"] = NapiHelper::Function::New<BigIntCallback>(env);
  obj["dateCallback"] = NapiHelper::Function::New<DateCallback>(env);
  obj["stringCallback"] = NapiHelper::Function::New<StringCallback>(env);
  obj["objectCallback"] = NapiHelper::Function::New<ObjectCallback>(env);
  obj["arrayCallback"] = NapiHelper::Function::New<ArrayCallback>(env);
  obj["symbolCallback"] = NapiHelper::Function::New<SymbolCallback>(env);
  obj["arrayBufferCallback"] =
      NapiHelper::Function::New<ArrayBufferCallback>(env);
  obj["typedArrayCallback"] =
      NapiHelper::Function::New<TypedArrayCallback>(env);
  obj["functionCallback"] = NapiHelper::Function::New<FunctionCallback>(env);
  obj["promiseCallback"] = NapiHelper::Function::New<PromiseCallback>(env);
  obj["dataViewCallback"] = NapiHelper::Function::New<DataViewCallback>(env);

  obj["external"] = Napi::External<void>::New(env, nullptr);
  obj["externalCallback"] = NapiHelper::Function::New<ExternalCallback>(env);

  obj["bufferCallback"] = NapiHelper::Function::New<BufferCallback>(env);

  obj["int8ArrayCallback"] = NapiHelper::Function::New<Int8ArrayCallback>(env);
  obj["uint8ArrayCallback"] =
      NapiHelper::Function::New<Uint8ArrayCallback>(env);
  obj["int16ArrayCallback"] =
      NapiHelper::Function::New<Int16ArrayCallback>(env);
  obj["uint16ArrayCallback"] =
      NapiHelper::Function::New<Uint16ArrayCallback>(env);
  obj["int32ArrayCallback"] =
      NapiHelper::Function::New<Int32ArrayCallback>(env);
  obj["uint32ArrayCallback"] =
      NapiHelper::Function::New<Uint32ArrayCallback>(env);
  obj["float32ArrayCallback"] =
      NapiHelper::Function::New<Float32ArrayCallback>(env);
  obj["float64ArrayCallback"] =
      NapiHelper::Function::New<Float64ArrayCallback>(env);
  obj["bigInt64ArrayCallback"] =
      NapiHelper::Function::New<BigInt64ArrayCallback>(env);
  obj["bigUint64ArrayCallback"] =
      NapiHelper::Function::New<BigUint64ArrayCallback>(env);

  obj["boolCallback"] = NapiHelper::Function::New<BoolCallback>(env);
  obj["doubleCallback"] = NapiHelper::Function::New<DoubleCallback>(env);
  obj["floatCallback"] = NapiHelper::Function::New<FloatCallback>(env);
  obj["uint8Callback"] = NapiHelper::Function::New<Uint8Callback>(env);
  obj["uint16Callback"] = NapiHelper::Function::New<Uint16Callback>(env);
  obj["uint32Callback"] = NapiHelper::Function::New<Uint32Callback>(env);
  obj["int8Callback"] = NapiHelper::Function::New<Int8Callback>(env);
  obj["int16Callback"] = NapiHelper::Function::New<Int16Callback>(env);
  obj["int32Callback"] = NapiHelper::Function::New<Int32Callback>(env);
  obj["int64Callback"] = NapiHelper::Function::New<Int64Callback>(env);
  obj["uint64Callback"] = NapiHelper::Function::New<Uint64Callback>(env);
  obj["strCallback"] = NapiHelper::Function::New<StrCallback>(env);
  obj["u16strCallback"] = NapiHelper::Function::New<U16StrCallback>(env);

  obj["vectorCallback"] = NapiHelper::Function::New<VectorCallback>(env);
  obj["tupleCallback"] = NapiHelper::Function::New<TupleCallback>(env);
  obj["functionWithVariants"] =
      NapiHelper::Function::New<FunctionWithVariants>(env);

  obj["voidCallback"] = NapiHelper::Function::New<VoidCallback>(env);

  obj["valueCallbackWithInfo"] =
      NapiHelper::Function::New<ValueCallbackWithInfo>(env);

  obj["lambda"] = NapiHelper::Function::New(
      env, [](int32_t num1) -> uint32_t { return num1 + 42; });

  obj["lambdaWithVoid"] =
      NapiHelper::Function::New(env, [](int32_t) -> void {});

  obj["lambdaWithCapture"] = NapiHelper::Function::New(
      env, [num = std::make_unique<uint32_t>(233)](int32_t num1) -> uint32_t {
        return num1 + *num;
      });

  obj["functionThrows"] = NapiHelper::Function::New<FunctionThrows>(env);
  obj["functionThrowsManually"] =
      NapiHelper::Function::New<FunctionThrowsManually>(env);

  return obj;
}
