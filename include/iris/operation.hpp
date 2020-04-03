#pragma once
#include <functional>
#include <iris/message.hpp>
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

} // namespace operation
} // namespace iris
