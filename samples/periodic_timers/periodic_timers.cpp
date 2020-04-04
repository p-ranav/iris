#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component c;
  c.set_interval(
      250, on_expiry = []() { std::cout << "Timer_1\n"; });
  c.set_interval(
      period = 500, on_expiry = []() { std::cout << "Timer_2\n"; });
  c.set_interval(
      period = 1000, on_expiry = []() { std::cout << "Timer_3\n"; });
  c.set_interval(
      period = 2000, on_expiry = []() { std::cout << "Timer_4\n"; });
  c.set_interval(
      period = 5000, on_expiry = []() { std::cout << "Timer_5\n"; });
  c.start();
}
