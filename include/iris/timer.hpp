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

template <typename P, typename T>
inline timer component::set_interval(P &&period_ms, T &&fn) {
  lock_t lock{timers_mutex_};
  auto t = std::make_unique<interval_timer>(
      std::forward<PeriodMs>(PeriodMs(period_ms)),
      operation::void_argument{.fn = TimerFunction(fn).get()}, executor_);
  interval_timers_.insert(std::make_pair(timer_count_.load(), std::move(t)));
  return timer(timer_count_++, this);
}

} // namespace iris