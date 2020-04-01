#include <iris/component.hpp>
#include <iris/timer.hpp>
#include <iostream>

int main() {
  iris::component timers;
  timers.set_interval(500, []() { std::cout << "Ping\n"; });
  timers.set_interval(1000, []() { std::cout << "Pong\n"; });
  timers.start();
}
