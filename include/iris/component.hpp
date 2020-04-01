#pragma once
#include <iris/task_system.hpp>
#include <iris/timer.hpp>
#include <iris/zmq_publisher.hpp>
#include <iris/zmq_subscriber.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

namespace iris {

class component {
  task_system executor_;
  std::unordered_map<std::string, std::unique_ptr<timer>> timers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<zmq_publisher>> publishers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<zmq_subscriber>>
      subscribers_;
  zmq::context_t context_{zmq::context_t(1)};
  std::mutex timers_mutex_, publishers_mutex_, subscribers_mutex_;

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
    timers_.clear();
  }

  void create_timer(std::string name, unsigned int period_ms,
                    std::function<void()> fn) {
    lock_t lock{timers_mutex_};
    auto t = std::make_unique<timer>(
        period_ms, operation::void_argument{.fn = fn}, executor_);
    timers_.insert(std::make_pair(std::move(name), std::move(t)));
  }

  void stop_timer(std::string timer_name) {
    lock_t lock{timers_mutex_};
    timers_[std::move(timer_name)]->stop();
  }

  class publisher create_publisher(std::vector<std::string> endpoints);

  class subscriber create_subscriber(std::vector<std::string> endpoints,
                                     std::function<void(std::string)> fn);

  void start() {
    executor_.start();
    for (auto &[_, v] : subscribers_) {
      v->start();
    }
    for (auto &[_, v] : timers_) {
      v->start();
    }
  }

  void stop() {
    executor_.stop();
    for (auto &[_, v] : subscribers_) {
      if (v)
        v->stop();
    }
    for (auto &[_, v] : timers_) {
      if (v)
        v->stop();
    }
  }
};

} // namespace iris
