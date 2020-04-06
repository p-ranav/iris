#include <iostream>
#include "numbers.hpp"
#include "statistics.hpp"
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component c(threads = 2);
  auto client = c.create_client(endpoints = {"tcp://localhost:5510"},
                                timeout = 2500, retries = 3);

  double i = 0.0, j = 1.0, k = 2.0;

  c.set_interval(
      period = 2000, on_triggered = [&] {
        std::cout << "[Sent] numbers = {" << i << ", " << j << ", " << k << "}\n";
        auto response = client.send(Numbers{.values = {i, j, k}});
        auto stats = response.get<Statistics>();
        std::cout << "[Received] mean = " << stats.mean 
                  << "; stdev = " << stats.stdev
                  << std::endl;
        i += 0.3;
        j += 0.5;
        k += 0.9;
      });
  c.start();
}