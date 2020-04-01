#include <iostream>
#include <iris/component.hpp>

class publisher : public iris::component {
  std::string message_;

public:
  publisher() : message_("Ping") {
    add_timer("timer_1", 50, std::bind(&publisher::on_timer_expiry, this));
    add_publisher("/my_data", {"tcp://*:5555"});
  }

  void on_timer_expiry() { publish("/my_data", "Hello"); }
};

int main() {
  publisher c;
  c.start();
}