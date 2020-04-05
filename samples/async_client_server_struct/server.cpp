#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct StatusCode {
  int value;

  template <typename Archive> void save(Archive &ar) const { ar(value); }
};

int main() {
  Component receiver(threads = 3);
  int code{0};
  receiver.create_broker(
    frontend = {"tcp://*5510"},
    backend = {"tcp://*:5515"}
  );
  receiver.create_async_server(
      endpoints = {"tcp://localhost:5515"}, timeout = 500,
      on_request = [&](Request request, Response &res) {
        std::cout << "Received: " << request.get<std::string>() << std::endl;
        std::cout << "Sending status code: " << code << "\n";
        res.set(StatusCode{.value = code++});
      });
  receiver.start();
}