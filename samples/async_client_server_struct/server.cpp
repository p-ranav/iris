#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component receiver(threads = 3);
  int code{0};
  receiver.create_async_server(
      endpoints = {"tcp://localhost:5515"}, timeout = 500,
      on_request = [&](Request request, Response &res) {
        res.set(code++);
      });
  receiver.start();
}