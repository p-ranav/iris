#include <iris/component.hpp>
#include <iris/timer.hpp>
#include <iostream>

int main() {
  iris::component timers;
  timers.set_interval(250, []() { std::cout << "Timer_1\n"; });
  timers.set_interval(500, []() { std::cout << "Timer_2\n"; });
  timers.set_interval(1000, []() { std::cout << "Timer_3\n"; });
  timers.set_interval(2000, []() { std::cout << "Timer_4\n"; });
  timers.set_interval(5000, []() { std::cout << "Timer_5\n"; });
  timers.start();
}
