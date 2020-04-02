#include <iris/component.hpp>
#include <iris/timer.hpp>
#include <iostream>

int main() {
  iris::component c;
  c.set_interval(period =  250, on_expiry = []() { std::cout << "Timer_1\n"; });
  c.set_interval(period =  500, on_expiry = []() { std::cout << "Timer_2\n"; });
  c.set_interval(period = 1000, on_expiry = []() { std::cout << "Timer_3\n"; });
  c.set_interval(period = 2000, on_expiry = []() { std::cout << "Timer_4\n"; });
  c.set_interval(period = 5000, on_expiry = []() { std::cout << "Timer_5\n"; });
  c.start();
}
