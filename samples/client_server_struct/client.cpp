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
  auto client =
      c.create_client(endpoints = {"tcp://127.0.0.1:5510"}, timeout = 2500);
  c.set_interval(
      period = 200, on_expiry = [&] {
        std::cout << "Sending Ping\n";
        auto response = client.send("Ping");
        std::cout << "Received status code: "
                  << response.get<StatusCode>().value << std::endl;
      });
  c.start();
}