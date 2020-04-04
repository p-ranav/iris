#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct LedRequest {
  // Request body
  unsigned index;
  bool state;

  // Request Deserialization
  template <typename Archive> void load(Archive &ar) { ar(index, state); }
};

int main() {
  std::array<bool, 3> led_panel{0, 0, 0};

  Component led_manager;
  led_manager.create_server(
      endpoints = {"tcp://*:5510"}, timeout = 500,
      on_request = [&](Request req) {
        LedRequest request = req.get<LedRequest>();
        led_panel[request.index] = request.state;
        std::cout << "Switched " << (request.state ? "ON" : "OFF") << " LED_"
                  << request.index << std::endl;
        return true;
      });
  led_manager.start();
}
