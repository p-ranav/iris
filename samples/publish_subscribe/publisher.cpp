#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>
#include <iris/timer.hpp>

class sender : public iris::component {
  iris::timer timer_;
  iris::publisher publisher_;

public:
  sender() {
    publisher_ = create_publisher({"tcp://*:5555"});

    set_interval(50, [this]() { publisher_.send("Hello, World!"); });
  }
};

int main() {
  sender s;
  s.start();
}