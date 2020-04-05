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

namespace iris {

namespace internal {

class OneShotTimerImpl {
  DelayMs delay_ms_;
  operation::TimerOperation fn_;
  std::reference_wrapper<TaskSystem> executor_;

public:
  template <typename P, typename T>
  OneShotTimerImpl(P &&delay_ms, T &&fn, TaskSystem &executor)
      : delay_ms_(delay_ms), fn_(fn), executor_(executor) {}

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

  std::thread start() {
    return std::thread([this]() {
      sleep_for(delay_ms_.get());
      executor_.get().async_(fn_);
    });
  }
};

} // namespace internal

} // namespace iris
