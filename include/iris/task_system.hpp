#pragma once
#include <atomic>
#include <iris/notification_queue.hpp>
#include <iris/operation.hpp>
#include <thread>
#include <vector>

namespace iris {

class TaskSystem {
  const unsigned count_{std::thread::hardware_concurrency()};
  std::vector<std::thread> threads_;
  std::vector<NotificationQueue> queue_{count_};
  std::atomic<unsigned> index_{0};
  std::atomic<bool> done_{0};
  friend class Component;

  void run(unsigned i) {
    while (!done_) {
      operation_t op;
      for (unsigned n = 0; n != count_; ++n) {
        if (queue_[(i + n) % count_].try_pop(op))
          break;
      }
      if (!valid_operation(op) && !queue_[i].try_pop(op))
        continue;
      if (auto void_op = std::get_if<operation::TimerOperation>(&op))
        (*void_op).fn();
      else if (auto subscriber_op =
                   std::get_if<operation::SubscriberOperation>(&op))
        (*subscriber_op).fn(std::move((*subscriber_op).arg));
    }
  }

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

  void stop() { done_ = true; }

  template <typename F> void async_(F &&f) {
    if (done_)
      return;
    auto i = index_++;
    for (unsigned n = 0; n != count_; ++n) {
      if (queue_[(i + n) % count_].try_push(std::forward<F>(f)))
        return;
    }
    queue_[i % count_].try_push(std::forward<F>(f));
  }
};

} // namespace iris
