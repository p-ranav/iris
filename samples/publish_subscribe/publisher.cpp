#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>

class publisher : public iris::component {
  iris::publisher pub_;

public:
  publisher() {
    create_timer("timer_1", 50, std::bind(&publisher::on_timer_expiry, this));
    pub_ = create_publisher({"tcp://*:5555"});
  }

  void on_timer_expiry() { pub_.send("Hello"); }
};

int main() {
  publisher c;
  c.start();
}