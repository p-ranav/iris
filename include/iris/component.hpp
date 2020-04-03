#pragma once
#include <initializer_list>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/internal/periodic_timer_impl.hpp>
#include <iris/internal/publisher_impl.hpp>
#include <iris/internal/subscriber_impl.hpp>
#include <iris/kwargs.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

namespace iris {

class Component {
  TaskSystem executor_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::PeriodicTimerImpl>>
      interval_timers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::PublisherImpl>>
      publishers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::SubscriberImpl>>
      subscribers_;
  zmq::context_t context_{zmq::context_t(1)};
  std::mutex timers_mutex_, publishers_mutex_, subscribers_mutex_;

  friend class PeriodicTimer;
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

  friend class Subscriber;
  std::atomic_uint8_t subscriber_count_{0};
  void stop_subscriber(std::uint8_t subscriber_id) {
    lock_t lock{subscribers_mutex_};
    subscribers_[subscriber_id]->stop();
  }

  friend class Message;
  template <typename T, typename U = std::string>
  T deserialize(std::uint8_t subscriber_id, U &&message) {
    lock_t lock{subscribers_mutex_};
    return subscribers_[subscriber_id]->deserialize<T>(
        std::forward<U>(message));
  }

public:
  Component() : executor_(TaskSystem(std::thread::hardware_concurrency())) {}

  template <typename T>
  Component(T &&n) : executor_(TaskSystem(Threads(n).get())) {}

  ~Component() {
    for (auto &thread : executor_.threads_)
      thread.join();
    subscribers_.clear();
    publishers_.clear();
    interval_timers_.clear();
  }

  template <typename P, typename T>
  class PeriodicTimer set_interval(P &&period_ms, T &&fn);

  template <typename E> class Publisher create_publisher(E &&endpoints);

  template <typename E, typename S>
  class Subscriber create_subscriber(E &&endpoints, S &&fn);

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
