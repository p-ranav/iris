#pragma once
#include <functional>
#include <string>
#include <variant>
#include <iris/subscriber_message.hpp>

namespace iris {

namespace operation {

struct void_argument {
  std::function<void()> fn;
};

struct subscriber_operation {
  std::function<void(subscriber_message)> fn;
  subscriber_message arg;
};

} // namespace operation
} // namespace iris
