#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});

  unsigned i{0};
  sender.set_interval(period = 250, 
    on_expiry = [&] { 
      const auto msg = "Hello World " + std::to_string(i);
      p.send(msg);
      std::cout << "Published " << msg << "\n";
      ++i;
  });
  sender.start();
}