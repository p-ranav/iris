#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct StatusCode {
  int value;

  template <typename Archive>
  void save(Archive& ar) const {
    ar(value);
  }
};

int main() {
  Component receiver(threads = 1);
  receiver.create_server(endpoints = {"tcp://*:5510"},
    timeout = -1,
    on_request = [](Request request) {
      std::cout << "Received: " << request.get<std::string>() << std::endl;
      return StatusCode{.value = 5};
  });
  receiver.start();
}