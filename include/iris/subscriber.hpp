#pragma once
#include <iostream>
#include <iris/component.hpp>

namespace iris {

class subscriber {
  friend component;
  std::uint8_t id_;
  component *component_;

  subscriber(std::uint8_t id, component *component)
      : id_(id), component_(component) {}

public:
  subscriber() = default;

  void stop() { component_->stop_subscriber(id_); }
};

inline subscriber
component::create_subscriber(std::vector<std::string> endpoints,
                             std::function<void(std::string)> fn) {
  lock_t lock{subscribers_mutex_};
  auto s = std::make_unique<zmq_subscriber>(
      context_, std::move(endpoints), /* filter */ "",
      operation::string_argument{.fn = fn}, executor_);
  subscribers_.insert(std::make_pair(subscriber_count_.load(), std::move(s)));
  return subscriber(subscriber_count_++, this);
}

} // namespace iris