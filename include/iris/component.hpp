#pragma once
#include <iris/publisher.hpp>
#include <iris/subscriber.hpp>
#include <iris/task_system.hpp>
#include <iris/timer.hpp>
#include <map>
#include <memory>
#include <vector>
#include <zmq.hpp>

namespace iris {

class component {
  task_system executor_;
  std::map<std::string, std::unique_ptr<timer>> timers_;
  std::map<std::string, std::unique_ptr<publisher>> publishers_;
  std::map<std::string, std::unique_ptr<subscriber>> subscribers_;
  zmq::context_t context_{zmq::context_t(1)};
  std::mutex timers_mutex_, publishers_mutex_, subscribers_mutex_;
  std::vector<std::thread *> interface_threads_;

public:
  component(const unsigned n = std::thread::hardware_concurrency())
      : executor_(task_system(n)) {}

  ~component() {
    subscribers_.clear();
    publishers_.clear();
    timers_.clear();
  }

  void add_timer(std::string name, unsigned int period,
                 std::function<void()> fn) {
    lock_t lock{timers_mutex_};
    auto t = std::make_unique<timer>(
        std::move(period), operation::void_argument{.fn = fn}, executor_);
    timers_.insert(std::make_pair(std::move(name), std::move(t)));
  }

  void stop_timer(std::string timer_name) {
    lock_t lock{timers_mutex_};
    timers_[std::move(timer_name)]->stop();
  }

  void add_publisher(std::string publisher_name,
                     std::vector<std::string> endpoints) {
    lock_t lock{publishers_mutex_};
    auto p =
        std::make_unique<publisher>(context_, std::move(endpoints), executor_);
    publishers_.insert(std::make_pair(std::move(publisher_name), std::move(p)));
  }

  void publish(std::string publisher_name, std::string message) {
    lock_t lock{publishers_mutex_};
    publishers_[std::move(publisher_name)]->send(std::move(message));
  }

  void add_subscriber(std::string subscriber_name,
                      std::vector<std::string> endpoints,
                      std::function<void(std::string)> fn) {
    lock_t lock{subscribers_mutex_};
    auto s = std::make_unique<subscriber>(context_, std::move(endpoints), "",
                                          operation::string_argument{.fn = fn},
                                          executor_);
    subscribers_.insert(
        std::make_pair(std::move(subscriber_name), std::move(s)));
  }

  void stop_subscriber(std::string subscriber_name) {
    lock_t lock{subscribers_mutex_};
    subscribers_[std::move(subscriber_name)]->stop();
  }

  void start() {
    executor_.start();
    for (auto &[_, v] : subscribers_) {
      interface_threads_.push_back(v->start());
    }
    for (auto &[_, v] : timers_) {
      v->start();
    }

    for (auto &thread : interface_threads_)
      thread->join();

    for (auto &thread : executor_.threads_)
      thread.join();
  }

  void stop() {
    executor_.done_ = true;
    for (auto &[_, v] : subscribers_) {
      v->stop();
    }
    for (auto &[_, v] : timers_) {
      v->stop();
    }
  }
};

} // namespace iris
