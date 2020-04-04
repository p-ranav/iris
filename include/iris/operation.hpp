#pragma once
#include <functional>
#include <iris/message.hpp>
#include <iris/request.hpp>
#include <iris/response.hpp>
#include <string>
#include <variant>

namespace iris {

namespace operation {

struct TimerOperation {
  std::function<void()> fn;
};

struct SubscriberOperation {
  std::function<void(Message)> fn;
  Message arg;
};

struct ServerOperation {
  std::function<void(Request, Response&)> fn;
  Request arg;
};

} // namespace operation
} // namespace iris
