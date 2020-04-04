#pragma once
#include <iostream>
#include <iris/component.hpp>
#include <iris/message.hpp>

namespace iris {

class Subscriber {
  friend Component;
  std::uint8_t id_;
  Component *component_;

  Subscriber(std::uint8_t id, Component *component)
      : id_(id), component_(component) {}

public:
  Subscriber() = default;

  void stop() { component_->stop_subscriber(id_); }
};

template <typename E, typename T, typename S>
inline Subscriber Component::create_subscriber(E &&endpoints, T &&timeout,
                                               S &&fn) {
  lock_t lock{subscribers_mutex_};
  auto s = std::make_unique<internal::SubscriberImpl>(
      subscriber_count_.load(), this, context_,
      std::forward<Endpoints>(Endpoints(endpoints)), /* filter */ "",
      std::forward<TimeoutMs>(TimeoutMs(timeout)),
      operation::SubscriberOperation{.fn = SubscriberFunction(fn).get()},
      executor_);
  subscribers_.insert(std::make_pair(subscriber_count_.load(), std::move(s)));
  return Subscriber(subscriber_count_++, this);
}

inline internal::SubscriberImpl::SubscriberImpl(
    std::uint8_t id, Component *parent, zmq::context_t &context,
    Endpoints endpoints, std::string filter, TimeoutMs timeout,
    const operation::SubscriberOperation &fn, TaskSystem &executor)
    : id_(id), component_(parent), context_(context),
      endpoints_(std::move(endpoints)), filter_(std::move(filter)), fn_(fn),
      executor_(executor) {
  socket_ = std::make_unique<zmq::socket_t>(context, ZMQ_SUB);
  for (auto &e : endpoints_) {
    socket_->connect(e);
  }
  socket_->setsockopt(ZMQ_SUBSCRIBE, filter_.c_str(), filter_.length());
  socket_->set(zmq::sockopt::rcvtimeo, timeout.get());
}

inline void internal::SubscriberImpl::recv() {
  while (!done_) {
    zmq::message_t received_message;
    socket_->recv(received_message);
    const auto message = std::string(
        static_cast<char *>(received_message.data()), received_message.size());
    if (message.length() > 0) {
      Message payload;
      payload.payload_ = std::move(message);
      payload.subscriber_id_ = id_;
      payload.component_ = component_;
      fn_.arg = std::move(payload);
      executor_.get().async_(fn_);
    }
  }
}

inline void internal::SubscriberImpl::start() {
  thread_ = std::thread(&SubscriberImpl::recv, this);
  started_ = true;
}

template <typename T> inline T Message::get() {
  return component_->get<T>(subscriber_id_, payload_);
}

} // namespace iris