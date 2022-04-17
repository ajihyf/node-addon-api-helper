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
      fun(NapiHelper::RangeError(num + "should be geq 3"), {});
    }
  }).detach();
}
}  // namespace

Napi::Object InitMultithread(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);

  obj["asyncNotify"] = NapiHelper::Function::New<AsyncNotify>(env);
  obj["asyncMinus3"] = NapiHelper::Function::New<AsyncMinus3>(env);

  return obj;
}
