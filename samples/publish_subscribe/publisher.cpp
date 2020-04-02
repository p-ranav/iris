#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>
#include <iris/timer.hpp>

int main() {
  iris::component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});
  sender.set_interval(period = 50,
                      on_expiry = [&p] { p.send("Hello"); });
  sender.start();

  iris::component sender2;
  auto p2 = sender2.create_publisher(endpoints = {"tcp://*:5556"});
  sender2.set_interval(period = 100,
                      on_expiry = [&p2] { p2.send("World!"); });
  sender2.start();
}