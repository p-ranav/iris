#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <mutex>
#include <ratio>
#include <thread>
#include <condition_variable>

namespace iris {

namespace internal {

class PeriodicTimerImpl {
  PeriodMs period_ms_;
  operation::TimerOperation fn_;
  std::reference_wrapper<TaskSystem> executor_;

  std::atomic<bool> execute_{false};
  std::thread thread_;

  std::mutex ready_mutex_;
  std::condition_variable ready_;

public:
  template <typename P, typename T>
  PeriodicTimerImpl(P &&period_ms, T &&fn, TaskSystem &executor)
      : period_ms_(period_ms), fn_(fn), executor_(executor), execute_(false),
        thread_({}) {}

  ~PeriodicTimerImpl() {
    if (execute_) {
      stop();
    };
  }

  void start() {
    if (execute_) {
      stop();
    };
    execute_ = true;
    thread_ = std::thread([this]() {
      while (execute_) {
        executor_.get().async_(fn_);
        lock_t lock{ready_mutex_};
        ready_.wait_for(lock, std::chrono::milliseconds(period_ms_.get()));
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

} // namespace internal

} // namespace iris
