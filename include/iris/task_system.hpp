#pragma once
#include <atomic>
#include <iris/notification_queue.hpp>
#include <iris/operation.hpp>
#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>

namespace iris {

class TaskSystem {
  const unsigned count_{std::thread::hardware_concurrency()};
  std::vector<std::thread> threads_;
  std::vector<NotificationQueue> queue_{count_};
  std::atomic<unsigned> index_{0};
  std::atomic_bool done_{0};
  std::mutex queue_mutex_;
  std::condition_variable ready_;

  friend class Component;

  void run(unsigned i);

  bool valid_operation(operation_t &op) {
    if (std::holds_alternative<operation::TimerOperation>(op)) {
      if (std::get<operation::TimerOperation>(op).fn) {
        return true;
      } else
        return false;
    } else if (std::holds_alternative<operation::SubscriberOperation>(op)) {
      if (std::get<operation::SubscriberOperation>(op).fn)
        return true;
      else
        return false;
    } else if (std::holds_alternative<operation::ServerOperation>(op)) {
      if (std::get<operation::ServerOperation>(op).fn)
        return true;
      else
        return false;
    } else
      return false;
  }

public:
  TaskSystem(const unsigned count) : count_(count) {}

  ~TaskSystem() {
    for (auto &queue : queue_)
      queue.done();
  }

  void start() {
    for (unsigned n = 0; n != count_; ++n) {
      threads_.emplace_back([&, n] { run(n); });
    }
  }

  void stop() { 
    done_ = true; 
    ready_.notify_all();
  }

  template <typename F> void async_(F &&f) {
    while (!done_) {
      auto i = index_++;
      for (unsigned n = 0; n != count_; ++n) {
        if (queue_[(i + n) % count_].try_push(std::forward<F>(f))) {
          ready_.notify_one();
          return;
        }
      }
      if (queue_[i % count_].try_push(std::forward<F>(f))) {
        ready_.notify_one();
        return;
      }
      index_ = 0;
    }
  }
};

} // namespace iris
