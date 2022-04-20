#ifndef SRC_NAAH_FRIEND_H_
#define SRC_NAAH_FRIEND_H_

namespace naah {
namespace details {
class Invoker;
}  // namespace details
}  // namespace naah

static void napi_helper_auto_register_function_();

#define NAAH_FRIEND                   \
  template <typename T>               \
  friend class naah::ScriptWrappable; \
  friend void ::napi_helper_auto_register_function_();

#endif  // SRC_NAAH_FRIEND_H_
