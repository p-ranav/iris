#pragma once
#include <iostream>
#include <iris/component.hpp>
#include <iris/kwargs.hpp>

namespace iris {

class Publisher {
  friend Component;
  std::uint8_t id_;
  Component *component_;

  Publisher(std::uint8_t id, Component *component)
      : id_(id), component_(component) {}

public:
  Publisher() = default;

  template <typename Message> void send(Message &&message) {
    component_->publish(id_, std::forward<Message>(message));
  }
};

template <typename E>
inline Publisher Component::create_publisher(E &&endpoints) {
  lock_t lock{publishers_mutex_};
  auto p = std::make_unique<zmq_publisher>(
      context_, std::forward<Endpoints>(Endpoints(endpoints)), executor_);
  publishers_.insert(std::make_pair(publisher_count_.load(), std::move(p)));
  return Publisher(publisher_count_++, this);
}

} // namespace iris