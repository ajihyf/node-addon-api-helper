#ifndef SRC_NAPI_HELPER_FRIEND_H_
#define SRC_NAPI_HELPER_FRIEND_H_

namespace NapiHelper {
namespace details {
class Invoker;
}  // namespace details
}  // namespace NapiHelper

static void napi_helper_auto_register_function_();

#define NAPI_HELPER_FRIEND                   \
  friend class NapiHelper::details::Invoker; \
  friend void ::napi_helper_auto_register_function_();

#endif  // SRC_NAPI_HELPER_FRIEND_H_
