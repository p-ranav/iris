#pragma once
#include <iostream>
#include <iris/component.hpp>

namespace iris {

class Broker {
  friend Component;
  std::uint8_t id_;
  Component *component_;

  Broker(std::uint8_t id, Component *component)
      : id_(id), component_(component) {}

public:
  Broker() = default;

  void stop() { component_->stop_broker(id_); }
};

template <typename E>
inline Broker Component::create_broker(E &&frontend_endpoints,
                                       E &&backend_endpoints) {
  lock_t lock{brokers_mutex_};
  auto p = std::make_unique<internal::BrokerImpl>(
      broker_count_.load(), this, context_,
      std::forward<Endpoints>(Endpoints(frontend_endpoints)),
      std::forward<Endpoints>(Endpoints(backend_endpoints)));
  brokers_.insert(std::make_pair(broker_count_.load(), std::move(p)));
  return Broker(broker_count_++, this);
}

} // namespace iris