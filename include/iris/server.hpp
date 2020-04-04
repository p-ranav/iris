#pragma once
#include <iostream>
#include <iris/component.hpp>
#include <iris/message.hpp>

namespace iris {

class Server {
  friend Component;
  std::uint8_t id_;
  Component *component_;

  Server(std::uint8_t id, Component *component)
      : id_(id), component_(component) {}

public:
  Server() = default;

  void stop() { component_->stop_server(id_); }
};

template <typename E, typename T, typename S>
inline Server Component::create_server(E &&endpoints, T &&timeout, S &&fn) {
  lock_t lock{servers_mutex_};
  auto s = std::make_unique<internal::ServerImpl>(
      server_count_.load(), this, context_,
      std::forward<Endpoints>(Endpoints(endpoints)),
      std::forward<TimeoutMs>(TimeoutMs(timeout)),
      operation::ServerOperation{.fn = ServerFunction(fn).get()}, executor_);
  servers_.insert(std::make_pair(server_count_.load(), std::move(s)));
  return Server(server_count_++, this);
}

template <typename E, typename T, typename S>
inline internal::ServerImpl::ServerImpl(std::uint8_t id, Component *parent,
                                        zmq::context_t &context, E &&endpoints,
                                        T &&timeout, S &&fn,
                                        TaskSystem &executor)
    : id_(id), component_(parent), context_(context),
      endpoints_(std::move(endpoints)), fn_(fn), executor_(executor) {
  socket_ = std::make_unique<zmq::socket_t>(context, ZMQ_REP);
  for (auto &e : endpoints_) {
    socket_->bind(e);
  }
  socket_->set(zmq::sockopt::rcvtimeo, timeout.get());
}

inline void internal::ServerImpl::recv() {
  while (!done_) {
    while (!ready_) {
    }
    zmq::message_t received_message;
    auto ret = socket_->recv(received_message);
    if (ret.has_value()) {
      ready_ = false;
      const auto message =
          std::string(static_cast<char *>(received_message.data()),
                      received_message.size());
      if (message.length() > 0) {
        Request payload;
        payload.payload_ = std::move(message);
        payload.server_id_ = id_;
        payload.component_ = component_;
        fn_.arg = std::move(payload);
        executor_.get().async_(fn_);
      } else {
        ready_ = true;
      }
    }
  }
}

inline void internal::ServerImpl::start() {
  thread_ = std::thread(&ServerImpl::recv, this);
  started_ = true;
  ready_ = true;
}

template <typename T> inline T Request::get() {
  return component_->get<T>(server_id_, payload_);
}

} // namespace iris