#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component b(threads = 0);
  b.create_broker(
    frontend = {"tcp://*:5510"},
    backend = {"tcp://*:5515"}
  );
  b.start();
}