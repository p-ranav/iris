#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component c(threads = 1);
  auto client = c.create_client(endpoints = {"tcp://127.0.0.1:5510"}, timeout = 100);
  c.set_interval(period = 500,
                 on_expiry = [&] { 
                    auto response = client.send("Ping");
                 });
  c.start();
}