#pragma once
#include <iostream>
#include <iris/component.hpp>

namespace iris {

class PeriodicTimer {
  friend Component;
  std::uint8_t id_;
  Component *component_;

  PeriodicTimer(std::uint8_t id, Component *component)
      : id_(id), component_(component) {}

public:
  PeriodicTimer() = default;

  void stop() { component_->stop_timer(id_); }
};

template <typename P, typename T>
inline PeriodicTimer Component::set_interval(P &&period_ms, T &&fn) {
  lock_t lock{timers_mutex_};
  auto t = std::make_unique<internal::PeriodicTimerImpl>(
      std::forward<PeriodMs>(PeriodMs(period_ms)),
      operation::TimerOperation{.fn = TimerFunction(fn).get()}, executor_);
  interval_timers_.insert(std::make_pair(timer_count_.load(), std::move(t)));
  return PeriodicTimer(timer_count_++, this);
}

} // namespace iris