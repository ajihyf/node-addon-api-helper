#include <napi_helper.h>

#include <thread>

namespace {
void AsyncNotify(uint32_t num, std::function<void(uint32_t)> fun) {
  for (uint32_t i = 0; i < num; i++) {
    std::thread([i, fun] {
      std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));
      fun(i);
    }).detach();
  }
}

void AsyncMinus3(uint32_t num,
                 std::function<void(std::optional<NapiHelper::RangeError>,
                                    std::optional<uint32_t>)>
                     fun) {
  std::thread([num, fun] {
    if (num >= 3) {
      fun({}, num - 3);
    } else {
      fun(NapiHelper::RangeError(std::to_string(num) + "should be geq 3"), {});
    }
  }).detach();
}

NapiHelper::AsyncWork<void> AsyncWorker(uint32_t sleep,
                                        std::function<void()> cb) {
  return [sleep, cb = std::move(cb)] {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    cb();
  };
}

NapiHelper::AsyncWork<NapiHelper::Result<uint32_t>> PromiseWorker(
    uint32_t num) {
  return [num]() -> NapiHelper::Result<uint32_t> {
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
    return num + 42;
  };
}

NapiHelper::AsyncWork<NapiHelper::Result<uint32_t, NapiHelper::RangeError>>
PromiseWorkerWithReject(uint32_t num) {
  return [num]() -> NapiHelper::Result<uint32_t, NapiHelper::RangeError> {
    if (num > 42) {
      return NapiHelper::RangeError(std::to_string(num) +
                                    " is greater than 42");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
    return num + 42;
  };
}
}  // namespace

Napi::Object InitMultithread(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  obj["asyncNotify"] = NapiHelper::Function::New<AsyncNotify>(env);
  obj["asyncMinus3"] = NapiHelper::Function::New<AsyncMinus3>(env);

  obj["asyncWorker"] = NapiHelper::Function::New<AsyncWorker>(env);
  obj["promiseWorker"] = NapiHelper::Function::New<PromiseWorker>(env);
  obj["promiseWorkerWithReject"] =
      NapiHelper::Function::New<PromiseWorkerWithReject>(env);

  return obj;
}
