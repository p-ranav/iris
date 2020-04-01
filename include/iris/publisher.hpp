#pragma once
#include <iostream>
#include <iris/component.hpp>

namespace iris {

class publisher {
  friend component;
  std::uint8_t id_;
  component *component_;

  publisher(std::uint8_t id, component *component)
      : id_(id), component_(component) {}

public:
  publisher() = default;

  template <typename Message> void send(Message &&message) {
    component_->publish(id_, std::forward<Message>(message));
  }
};

inline publisher
component::create_publisher(std::vector<std::string> endpoints) {
  lock_t lock{publishers_mutex_};
  auto p = std::make_unique<zmq_publisher>(context_, std::move(endpoints),
                                           executor_);
  publishers_.insert(std::make_pair(publisher_count_.load(), std::move(p)));
  return publisher(publisher_count_++, this);
}

} // namespace iris