#include <iostream>
#include <iris/iris.hpp>

int main() {
  iris::Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
                             on_receive = [&](iris::Message msg) {
                                 std::cout << "Received "
                                           << msg.deserialize<std::string>()
                                           << "\n";
                             });
  receiver.start();
}