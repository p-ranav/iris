#include "numbers.hpp"
#include "statistics.hpp"
#include <iostream>
#include <iris/iris.hpp>
using namespace iris;
#include <algorithm>
#include <cmath>
#include <numeric>

int main() {
  Component receiver(threads = 3);

  receiver.create_async_server(
      endpoints = {"tcp://localhost:5515"}, timeout = 500,
      on_request = [&](Request request, Response &res) {
        auto numbers = request.get<Numbers>();

        std::cout << "Received numbers: {" << numbers.values[0] << ", "
                  << numbers.values[1] << ", " << numbers.values[2] << "}\n";

        // Calculate mean
        double sum = std::accumulate(numbers.begin(), numbers.end(), 0.0);
        double mean = sum / numbers.size();

        // Calculate standard deviation
        double accum = 0.0;
        std::for_each(numbers.begin(), numbers.end(), [&](const double d) {
          accum += (d - mean) * (d - mean);
        });
        double stdev = std::sqrt(accum / numbers.size());

        // Set the response
        res.set(Statistics{.mean = mean, .stdev = stdev});

        std::cout << "Calculated stats successfully\n";
      });
  receiver.start();
}