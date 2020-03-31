#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <mutex>
#include <ratio>
#include <thread>

namespace iris {

class timer {
  unsigned int period_;
  operation::void_argument fn_;
  std::reference_wrapper<task_system> executor_;

  std::atomic<bool> execute_{false};
  std::thread thread_;

public:
  timer(unsigned int period, const operation::void_argument &fn,
        task_system &executor)
      : period_(period), fn_(fn), executor_(executor), execute_(false),
        thread_({}) {}

  ~timer() {
    if (execute_) {
      stop();
    };
  }

  typedef std::chrono::high_resolution_clock clock;
  typedef std::chrono::milliseconds milliseconds;

  static void sleep_for(double delta) {
    static constexpr milliseconds min_sleep_duration(0);
    auto start = clock::now();
    while (
        std::chrono::duration_cast<milliseconds>(clock::now() - start).count() <
        delta) {
      std::this_thread::sleep_for(min_sleep_duration);
    }
  }

  void start() {
    if (execute_) {
      stop();
    };
    execute_ = true;
    thread_ = std::thread([this]() {
      while (execute_) {
        executor_.get().async_(fn_);
        sleep_for(period_);
      }
    });
  }

  void stop() {
    execute_ = false;
    if (thread_.joinable())
      thread_.join();
  }

  bool is_running() const noexcept { return (execute_ && thread_.joinable()); }
};

} // namespace iris
