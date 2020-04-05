#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct StatusCode {
  int value;

  // Method for deserialization
  template <typename Archive> void load(Archive &ar) { ar(value); }
};

int main() {
  Component c(threads = 2);
  auto client = c.create_client(endpoints = {"tcp://localhost:5510"},
                                timeout = 2500, retries = 3);
  // TODO: Consider adding a failure callback
  c.set_interval(
      period = 50, on_triggered = [&] {
        std::cout << "Sending Ping\n";
        auto response = client.send("Ping");
        std::cout << "Received status code: "
                  << response.get<StatusCode>().value << std::endl;
      });
  c.start();
}