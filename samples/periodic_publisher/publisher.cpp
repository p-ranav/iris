#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

class MyPublisher : public Component {
  std::atomic_uint8_t count_{0};
  Publisher pub_;

public:
  MyPublisher() {
    pub_ = create_publisher(endpoints = {"tcp://*:5555"});
    set_interval(period = 50,
                 on_expiry = std::bind(&MyPublisher::publish, this));
  }

  void publish() {
    pub_.send("Message " + std::to_string(count_));
    if (count_++ == 100)
      stop();
  }
};

int main() {
  MyPublisher p;
  p.start();
}