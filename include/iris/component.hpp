#pragma once
#include <iris/interval_timer.hpp>
#include <iris/kwargs.hpp>
#include <iris/task_system.hpp>
#include <iris/zmq_publisher.hpp>
#include <iris/zmq_subscriber.hpp>
#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

namespace iris {

class component {
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

  friend class publisher;
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

public:
  component(const unsigned n = std::thread::hardware_concurrency())
      : executor_(task_system(n)) {}

  ~component() {
    for (auto &thread : executor_.threads_)
      thread.join();
    subscribers_.clear();
    publishers_.clear();
    interval_timers_.clear();
  }

  class timer set_interval(PeriodMs period_ms, TimerFunction fn);

  class publisher create_publisher(std::vector<std::string> endpoints);

  class subscriber create_subscriber(std::vector<std::string> endpoints,
                                     SubscriberFunction fn);

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
