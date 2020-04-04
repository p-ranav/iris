#include <iostream>
#include <iris/iris.hpp>
#include <random>
using namespace iris;

struct Mouse {
  int x, y;

  // Method for serialization
  template <typename Archive> void save(Archive &ar) const { ar(x, y); }
};

int main() {
  std::random_device rd;  // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<> x_dist(0, 1920), y_dist(0, 1080);

  Component sender(threads = 1);
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});

  sender.set_interval(
      period = 250, on_expiry = [&] {
        Mouse position{.x = x_dist(eng), .y = y_dist(eng)};

        p.send(position);
        std::cout << "Published (" << position.x << ", " << position.y << ")\n";
      });
  sender.start();
}