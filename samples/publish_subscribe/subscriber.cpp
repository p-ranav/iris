#include <iostream>
#include <iris/component.hpp>
#include <iris/subscriber.hpp>

int main() {
  iris::component receiver(threads = 2);
  receiver.create_subscriber(
      endpoints = {"tcp://localhost:5555"},
      on_receive = [](auto msg) { std::cout << "Received " << msg << "\n"; });
  receiver.start();
}