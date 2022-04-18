# Async Work

Node.js has builtin worker threads, **naah** provides `AsyncWork` to automatically dispatch tasks to worker threads.

## Normal Work

Normal works without return value can be created by `naah::AsyncWork<void>` :

```cpp
naah::AsyncWork<void> AsyncWorker(uint32_t sleep,
                                        std::function<void()> cb) {
  return [sleep, cb = std::move(cb)] {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    cb();
  };
}
```

## Return Promise

To return a promise, use `naah::AsyncWork<naah::Result<ResolveType, RejectType>>` :

```cpp
naah::AsyncWork<naah::Result<uint32_t, naah::RangeError>>
PromiseWorkerWithReject(uint32_t num) {
  return [num]() -> naah::Result<uint32_t, naah::RangeError> {
    if (num > 42) {
      return naah::RangeError(num + " is greater than 42");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(num));
    return num + 42;
  };
}
```
