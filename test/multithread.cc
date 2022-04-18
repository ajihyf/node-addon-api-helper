#include <naah.h>

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
                 std::function<void(std::optional<naah::RangeError>,
                                    std::optional<uint32_t>)>
                     fun) {
  std::thread([num, fun] {
    if (num >= 3) {
      fun({}, num - 3);
    } else {
      fun(naah::RangeError(std::to_string(num) + "should be geq 3"), {});
    }
  }).detach();
}

naah::AsyncWork<void> AsyncWorker(uint32_t sleep, std::function<void()> cb) {
  return [sleep, cb = std::move(cb)] {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    cb();
  };
}

naah::AsyncWork<naah::Result<uint32_t>> PromiseWorker(uint32_t num) {
  return [num]() -> naah::Result<uint32_t> {
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
    return num + 42;
  };
}

naah::AsyncWork<naah::Result<uint32_t, naah::RangeError>>
PromiseWorkerWithReject(uint32_t num) {
  return [num]() -> naah::Result<uint32_t, naah::RangeError> {
    if (num > 42) {
      return naah::RangeError(std::to_string(num) + " is greater than 42");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
    return num + 42;
  };
}
}  // namespace

Napi::Object InitMultithread(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  obj["asyncNotify"] = naah::Function::New<AsyncNotify>(env);
  obj["asyncMinus3"] = naah::Function::New<AsyncMinus3>(env);

  obj["asyncWorker"] = naah::Function::New<AsyncWorker>(env);
  obj["promiseWorker"] = naah::Function::New<PromiseWorker>(env);
  obj["promiseWorkerWithReject"] =
      naah::Function::New<PromiseWorkerWithReject>(env);

  return obj;
}
