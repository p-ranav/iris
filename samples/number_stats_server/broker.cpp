#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component b(threads = 0);
  b.create_broker(router_endpoints = {"tcp://*:5510"},
                  dealer_endpoints = {"tcp://*:5515"});
  b.start();
}