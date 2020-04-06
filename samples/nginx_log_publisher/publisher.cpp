#include <iostream>
#include <iris/iris.hpp>
using namespace iris;
#include "nginx_log_entry.hpp"
#include "json.hpp"
#include <fstream>

class NginxLogPublisher : public Component {
  Publisher pub;
  nlohmann::json j;
  nlohmann::json::iterator it;

public:
  NginxLogPublisher(const std::string &filename) {
    // read a JSON file
    std::ifstream stream("nginx_logs.json");
    stream >> j;
    it = j.begin();

    // Craete publisher
    pub = create_publisher(endpoints = {"tcp://*:5555"});

    // Publish periodically
    set_interval( period = 50, 
                  on_triggered = [this] {
                      auto element = *it;
                      std::cout << "Published: " << element << std::endl;
                      pub.send(NginxLogEntry{
                        .time = element["time"].get<std::string>(),
                        .remote_ip = element["remote_ip"].get<std::string>(),
                        .remote_user = element["remote_user"].get<std::string>(),
                        .request = element["request"].get<std::string>(),
                        .response = element["response"].get<unsigned>(),
                        .bytes = element["bytes"].get<unsigned>(),
                        .agent = element["agent"].get<std::string>()
                      });
                      ++it;
                  });
  }

  ~NginxLogPublisher() {
    join();
  }

};

int main() {
  NginxLogPublisher publisher("nginx_logs.json");
  publisher.start();
}