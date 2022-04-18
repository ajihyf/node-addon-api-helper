# Async Work

Node.js has builtin worker threads, **node-addon-api-helper** provides `AsyncWork` to automatically dispatch tasks to worker threads.

## Normal Work

Normal works without return value can be created by `NapiHelper::AsyncWork<void>` :

```cpp
NapiHelper::AsyncWork<void> AsyncWorker(uint32_t sleep,
                                        std::function<void()> cb) {
  return [sleep, cb = std::move(cb)] {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    cb();
  };
}
```

## Return Promise

To return a promise, use `NapiHelper::AsyncWork<NapiHelper::Result<ResolveType, RejectType>>` :

```cpp
NapiHelper::AsyncWork<NapiHelper::Result<uint32_t, NapiHelper::RangeError>>
PromiseWorkerWithReject(uint32_t num) {
  return [num]() -> NapiHelper::Result<uint32_t, NapiHelper::RangeError> {
    if (num > 42) {
      return NapiHelper::RangeError(num + " is greater than 42");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
    return num + 42;
  };
}
```
