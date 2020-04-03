#include <iostream>
#include <iris/iris.hpp>

struct Foo {
  uint64_t value_{15};
  template <class Archive> void load(Archive &ar) { ar(value_); }
};

int main() {
  iris::Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
                             on_receive = [&](iris::Message msg) {
                               auto foo = msg.deserialize<Foo>();
                               std::cout << foo.value_ << std::endl;
                               // auto foo = receiver.deserialize<Foo>(msg);
                               // std::cout << "Received " << msg << " " <<
                               // foo.value_ << "\n";
                             });
  receiver.start();

  iris::Component receiver2(threads = 2);
  receiver2.create_subscriber(endpoints = {"tcp://localhost:5556"},
                              on_receive = [&](iris::Message msg) {
                                auto foo = msg.deserialize<std::string>();
                                std::cout << foo << std::endl;
                                // auto foo = receiver.deserialize<Foo>(msg);
                                // std::cout << "Received " << msg << " " <<
                                // foo.value_ << "\n";
                              });
  receiver2.start();
}