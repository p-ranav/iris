#include <iostream>
#include <iris/iris.hpp>
using namespace iris;
#include "nginx_log_entry.hpp"

int main() {
  Component receiver(threads = 2);
  receiver.create_subscriber(
      endpoints = {"tcp://localhost:5555"}, timeout = 5000,
      on_receive = [](Message msg) {
        auto entry = msg.get<NginxLogEntry>();
        std::cout << "[" << entry.time << "] "
                  << "{" << entry.remote_ip << "} "
                  << "-> " << entry.request << "-> " << entry.response
                  << std::endl;
      });
  receiver.start();
}