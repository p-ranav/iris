#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
    on_receive = [](Message msg) {
      std::cout << "Received "
                << msg.get<std::string>()
                << "\n";
  });
  receiver.start();
}