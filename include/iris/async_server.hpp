#pragma once
#include <iostream>
#include <iris/component.hpp>
#include <iris/message.hpp>

namespace iris {

class AsyncServer {
  friend Component;
  std::uint8_t id_;
  Component *component_;

  AsyncServer(std::uint8_t id, Component *component)
      : id_(id), component_(component) {}

public:
  AsyncServer() = default;

  void stop() { component_->stop_async_server(id_); }
};

template <typename E, typename T, typename S>
inline AsyncServer Component::create_async_server(E &&endpoints, T &&timeout, S &&fn) {
  lock_t lock{async_servers_mutex_};
  auto s = std::make_unique<internal::AsyncServerImpl>(
      async_server_count_.load(), this, context_,
      std::forward<Endpoints>(Endpoints(endpoints)),
      std::forward<TimeoutMs>(TimeoutMs(timeout)),
      operation::ServerOperation{.fn = ServerFunction(fn).get()}, executor_);
  async_servers_.insert(std::make_pair(async_server_count_.load(), std::move(s)));
  return AsyncServer(async_server_count_++, this);
}

template <typename E, typename T, typename S>
inline internal::AsyncServerImpl::AsyncServerImpl(std::uint8_t id, Component *parent,
                                        zmq::context_t &context, E &&endpoints,
                                        T &&timeout, S &&fn,
                                        TaskSystem &executor)
    : id_(id), component_(parent), context_(context),
      endpoints_(std::move(endpoints)), fn_(fn), executor_(executor) {
  socket_ = std::make_unique<zmq::socket_t>(context, ZMQ_REP);
  for (auto &e : endpoints_) {
    socket_->connect(e);
  }
  socket_->set(zmq::sockopt::rcvtimeo, timeout.get());
}

inline void internal::AsyncServerImpl::recv() {
  while (!done_) {
    while (!ready_) {
    }
    zmq::message_t message;
    auto ret = socket_->recv(message);
    if (ret.has_value()) {
      ready_ = false;
      Request payload;
      payload.payload_ = std::move(message);
      payload.server_id_ = id_;
      payload.component_ = component_;
      payload.async_ = true;
      fn_.arg = payload;
      executor_.get().async_(fn_);
    }
  }
}

inline void internal::AsyncServerImpl::start() {
  thread_ = std::thread(&AsyncServerImpl::recv, this);
  started_ = true;
  ready_ = true;
}

} // namespace iris