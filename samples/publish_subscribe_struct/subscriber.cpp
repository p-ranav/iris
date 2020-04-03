#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct Mouse {
  int x, y;

  // Method for deserialization
  template <typename Archive>
  void load(Archive& ar) {
    ar(x, y);
  }
};

int main() {
  Component receiver(threads = 1);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
    timeout = 100,
    on_receive = [](Message msg) {
      auto position = msg.get<Mouse>();
      std::cout << "Received ("
        << position.x << ", " << position.y << ")\n";
  });
  receiver.start();
}