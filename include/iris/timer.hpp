#pragma once
#include <iostream>
#include <iris/component.hpp>

namespace iris {

class timer {
  friend component;
  std::uint8_t id_;
  component *component_;

  timer(std::uint8_t id, component *component)
      : id_(id), component_(component) {}

public:
  timer() = default;

  void stop() { component_->stop_timer(id_); }
};

inline timer component::set_interval(unsigned int period_ms,
                                     std::function<void()> fn) {
  return set_interval(PeriodMs(period_ms), fn);
}

inline timer component::set_interval(PeriodMs period_ms,
                                     std::function<void()> fn) {
  lock_t lock{timers_mutex_};
  auto t = std::make_unique<interval_timer>(
      period_ms, operation::void_argument{.fn = fn}, executor_);
  interval_timers_.insert(std::make_pair(timer_count_.load(), std::move(t)));
  return timer(timer_count_++, this);
}

} // namespace iris