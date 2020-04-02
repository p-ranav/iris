#pragma once
#include <functional>
#include <string>
#include <variant>
#include <iris/message.hpp>

namespace iris {

namespace operation {

struct void_argument {
  std::function<void()> fn;
};

struct subscriber_operation {
  std::function<void(Message)> fn;
  Message arg;
};

} // namespace operation
} // namespace iris
