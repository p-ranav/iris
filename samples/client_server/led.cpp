#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct SetLED {
  // Request body
  unsigned index;
  bool state;

  // Request Deserialization
  template <typename Archive> void load(Archive &ar) { ar(index, state); }
};

int main() {
  std::array<bool, 3> led_panel{0, 0, 0};

  Component led_manager(threads = 1);
  led_manager.create_server(
      endpoints = {"tcp://*:5510"}, timeout = 500,
      on_request = [&](Request req) {
        auto request = req.get<SetLED>();
        led_panel[request.index] = request.state;
        std::cout << "Switched " << (request.state ? "ON" : "OFF") << " LED_"
                  << request.index << std::endl;
        return true;
      });
  led_manager.start();
}
