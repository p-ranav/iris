#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component receiver(threads = 1);
  receiver.create_server(endpoints = {"tcp://*:5510"},
    timeout = 100,
    on_request = [](Request request) {
      return Response("Pong");
  });
  receiver.start();
}