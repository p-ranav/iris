#include <iostream>
#include <iris/component.hpp>

class subscriber : public iris::component {
public:
  subscriber() : component() {
    add_subscriber(
        "subscriber_1", {"tcp://localhost:5555", "tcp://localhost:5556"},
        std::bind(&subscriber::on_receive, this, std::placeholders::_1));
  }

  void on_receive(std::string message) {
    std::cout << "Received " << message << "\n";
  }
};

int main() {
  subscriber c;
  c.start();
}