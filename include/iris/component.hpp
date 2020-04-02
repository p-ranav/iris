#pragma once
#include <initializer_list>
#include <iris/interval_timer.hpp>
#include <iris/kwargs.hpp>
#include <iris/task_system.hpp>
#include <iris/zmq_publisher.hpp>
#include <iris/zmq_subscriber.hpp>
#include <iris/cereal/archives/portable_binary.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>
#include <sstream>

namespace iris {

class Component {
  task_system executor_;
  std::unordered_map<std::uint8_t, std::unique_ptr<interval_timer>>
      interval_timers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<zmq_publisher>> publishers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<zmq_subscriber>>
      subscribers_;
  zmq::context_t context_{zmq::context_t(1)};
  std::mutex timers_mutex_, publishers_mutex_, subscribers_mutex_;

  friend class timer;
  std::atomic_uint8_t timer_count_{0};
  void stop_timer(std::uint8_t timer_id) {
    lock_t lock{timers_mutex_};
    interval_timers_[timer_id]->stop();
  }

  friend class Publisher;
  std::atomic_uint8_t publisher_count_{0};
  template <typename Message>
  void publish(std::uint8_t publisher_id, Message &&message) {
    lock_t lock{publishers_mutex_};
    publishers_[publisher_id]->send(std::forward<Message>(message));
  }

  friend class subscriber;
  std::atomic_uint8_t subscriber_count_{0};
  void stop_subscriber(std::uint8_t subscriber_id) {
    lock_t lock{subscribers_mutex_};
    subscribers_[subscriber_id]->stop();
  }

  friend class subscriber_message;
  template <typename T, typename U = std::string>
  T deserialize(std::uint8_t subscriber_id, U&& message) {
    lock_t lock{subscribers_mutex_};
    return subscribers_[subscriber_id]->deserialize<T>(std::forward<U>(message));
  }

public:
  Component() : executor_(task_system(std::thread::hardware_concurrency())) {}

  template <typename T>
  Component(T &&n) : executor_(task_system(Threads(n).get())) {}

  ~Component() {
    for (auto &thread : executor_.threads_)
      thread.join();
    subscribers_.clear();
    publishers_.clear();
    interval_timers_.clear();
  }

  template <typename P, typename T>
  class timer set_interval(P &&period_ms, T &&fn);

  template <typename E> class Publisher create_publisher(E &&endpoints);

  template <typename E, typename S>
  class subscriber create_subscriber(E &&endpoints, S &&fn);

  void start() {
    executor_.start();
    for (auto &[_, v] : subscribers_) {
      v->start();
    }
    for (auto &[_, v] : interval_timers_) {
      v->start();
    }
  }

  void stop() {
    executor_.stop();
    for (auto &[_, v] : subscribers_) {
      if (v)
        v->stop();
    }
    for (auto &[_, v] : interval_timers_) {
      if (v)
        v->stop();
    }
  }
};

} // namespace iris
