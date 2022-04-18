# Thread Safe Function

Thread safety in node C++ addon development is complex.
You should make sure no data race, never call JavaScript from non-JS thread, dispatch tasks to JS-thread from non-JS threads, clean JavaScript resources when environment shutdowns, etc.

Luckily, in node-addon-api, there are [Napi::ThreadSafeFunction](https://github.com/nodejs/node-addon-api/blob/main/doc/threadsafe.md)s to help to keep thread safety. However, the usage of `Napi::ThreadSafeFunction` may be too advanced in most scenarios. **naah** gives you a simple-to-use version of thread safe function: just use `std::function`!

All input arguments with signature `std::function<void(Args...)>` are automatically transformed from JavaScript functions, but safe to call in any thread.

```cpp
void asyncMinus3(uint32_t num,
                 std::function<void(std::optional<naah::RangeError>,
                                    std::optional<uint32_t>)>
                     cb) {
  std::thread([num, cb] {
    if (num >= 3) {
      cb({}, num - 3);
    } else {
      cb(naah::RangeError(num + "should be geq 3"), {});
    }
  }).detach();
}
```

In JavaScript land :

```javascript
binding.asyncMinus3(num, (err, v) => {
  if (err) {
    // handle error
  } else {
    // v is num - 3
  }
});
```
