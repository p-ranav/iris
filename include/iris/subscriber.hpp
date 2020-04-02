#pragma once
#include <iostream>
#include <iris/component.hpp>
#include <iris/subscriber_message.hpp>

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

template <typename E, typename S>
inline subscriber component::create_subscriber(E &&endpoints, S &&fn) {
  lock_t lock{subscribers_mutex_};
  auto s = std::make_unique<zmq_subscriber>(
      subscriber_count_.load(), this, context_, std::forward<Endpoints>(Endpoints(endpoints)), /* filter */ "",
      operation::subscriber_operation{.fn = SubscriberFunction(fn).get()},
      executor_);
  subscribers_.insert(std::make_pair(subscriber_count_.load(), std::move(s)));
  return subscriber(subscriber_count_++, this);
}

inline 
zmq_subscriber::zmq_subscriber(std::uint8_t id, component * parent, zmq::context_t &context, Endpoints endpoints,
                 std::string filter, const operation::subscriber_operation &fn,
                 task_system &executor)
      : id_(id), component_(parent), context_(context), endpoints_(std::move(endpoints)),
        filter_(std::move(filter)), fn_(fn), executor_(executor) {
    socket_ = std::make_unique<zmq::socket_t>(context, ZMQ_SUB);
    for (auto &e : endpoints_) {
      socket_->connect(e);
    }
    socket_->setsockopt(ZMQ_SUBSCRIBE, filter_.c_str(), filter_.length());
    socket_->setsockopt(ZMQ_RCVTIMEO, 0);
  }

inline 
  void zmq_subscriber::recv() {
    while (!done_) {
      zmq::message_t received_message;
      socket_->recv(&received_message);
      const auto message =
          std::string(static_cast<char *>(received_message.data()),
                      received_message.size());
      if (message.length() > 0) {
        subscriber_message payload;
        payload.payload_ = std::move(message);
        payload.subscriber_id_ = id_;
        payload.component_ = component_;
        fn_.arg = std::move(payload);
        executor_.get().async_(fn_);
      }
    }
  }

inline 
  void zmq_subscriber::start() { 
    thread_ = std::thread(&zmq_subscriber::recv, this);
    started_ = true;
  }
 
    template <typename T>
    inline T subscriber_message::deserialize() {
        return component_->deserialize<T>(subscriber_id_, payload_);
    }

} // namespace iris