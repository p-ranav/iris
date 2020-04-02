#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>
#include <iris/timer.hpp>

class my_publisher: public iris::component {
  std::atomic_uint8_t count_{0};
  iris::publisher pub_;
public:
  my_publisher() {
    pub_ = create_publisher(endpoints = {"tcp://*:5555"});
    set_interval(period = 50, on_expiry = std::bind(&my_publisher::publish, this));
  }

  void publish() {
    pub_.send("Message " + std::to_string(count_));
    if (count_++ == 100)
      stop();
  }
};

int main() {
  my_publisher p;
  p.start();
}