#include <iostream>
#include <iris/component.hpp>
#include <iris/subscriber.hpp>

struct Foo {
  uint64_t value_{15};   
  template <class Archive>
  void load(Archive & ar) {
    ar(value_);
  }
};

int main() {
  iris::component receiver(threads = 2);
  receiver.create_subscriber(
      endpoints = {"tcp://localhost:5555"},
      on_receive = [&](iris::subscriber_message msg) { 
        auto foo = msg.deserialize<Foo>();
        std::cout << foo.value_ << std::endl;
        // auto foo = receiver.deserialize<Foo>(msg);
        // std::cout << "Received " << msg << " " << foo.value_ << "\n"; 
      });
  receiver.start();

  iris::component receiver2(threads = 2);
  receiver2.create_subscriber(
      endpoints = {"tcp://localhost:5556"},
      on_receive = [&](iris::subscriber_message msg) { 
        auto foo = msg.deserialize<std::string>();
        std::cout << foo << std::endl;
        // auto foo = receiver.deserialize<Foo>(msg);
        // std::cout << "Received " << msg << " " << foo.value_ << "\n"; 
      });
  receiver2.start();

} 