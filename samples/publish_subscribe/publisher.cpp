#include <iostream>
#include <iris/iris.hpp>

struct Foo {
  uint64_t value_{15};
  template <class Archive>
  void save(Archive & ar) const {
    ar(value_);
  }
};

int main() {
  Foo foo;

  iris::Component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});
  sender.set_interval(500, [&p, &foo] { p.send(foo); });
  sender.start();

  iris::Component str_sender;
  auto p2 = str_sender.create_publisher(endpoints = {"tcp://*:5556"});
  str_sender.set_interval(500, [&p2] { p2.send(std::string{"Hello World"}); });
  str_sender.start();
}