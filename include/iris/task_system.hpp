#pragma once
#include <atomic>
#include <iris/notification_queue.hpp>
#include <iris/operation.hpp>
#include <thread>
#include <vector>
#include <iostream>

namespace iris {

class task_system {
  const unsigned count_{std::thread::hardware_concurrency()};
  std::vector<std::thread> threads_;
  std::vector<notification_queue> queue_{count_};
  std::atomic<unsigned> index_{0};
  friend class component;

  bool valid_operation(operation_t &op) {
    if (std::holds_alternative<operation::void_argument>(op)) {
      if (std::get<operation::void_argument>(op).fn) {
        return true;
      } else return false;
    }
    else if (std::holds_alternative<operation::string_argument>(op)) {
      if (std::get<operation::string_argument>(op).fn)
        return true;
      else return false;
    }
  }

  void run(unsigned i) {
    while (true) {
      operation_t op;
      for (unsigned n = 0; n != count_; ++n) {
        if (queue_[(i + n) % count_].try_pop(op))
          continue; // break;
      }
      if (!valid_operation(op) && !queue_[i].try_pop(op))
        continue; // break;
      if (auto void_op = std::get_if<operation::void_argument>(&op))
        (*void_op).fn();
      else if (auto string_op = std::get_if<operation::string_argument>(&op))
        (*string_op).fn(std::move((*string_op).arg));
    }    
  }

public:
  task_system() {
    for (unsigned n = 0; n != count_; ++n) {
      threads_.emplace_back([&, n] { run(n); });
    }
  }

  ~task_system() {
    std::cout << "Task system destructor called\n";
    for (auto &queue : queue_)
      queue.done();
    for (auto &thread : threads_)
      thread.join();
    std::cout << "Task system destructed!\n";
  }

  template <typename F> void async_(F &&f) {
    auto i = index_++;
    for (unsigned n = 0; n != count_; ++n) {
      if (queue_[(i + n) % count_].try_push(std::forward<F>(f)))
        return;
    }
    queue_[i % count_].try_push(std::forward<F>(f));
  }  
};

}
