#pragma once
#include <atomic>
#include <functional>
#include <iris/cereal/archives/json.hpp>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/cppzmq/zmq.hpp>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace iris {

namespace internal {

class ServerImpl {
  std::uint8_t id_;
  class Component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<TaskSystem> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;
  operation::ServerOperation fn_;
  std::thread thread_;
  std::atomic_bool started_{false};
  std::atomic_bool done_{false};

  std::mutex ready_mutex_;
  std::condition_variable ready_;

public:
  template <typename E, typename T, typename S>
  ServerImpl(std::uint8_t id, Component *parent, zmq::context_t &context,
             E &&endpoints, T &&timeout, S &&fn, TaskSystem &executor);

  ~ServerImpl() {
    if (started_)
      if (thread_.joinable())
        thread_.join();
    socket_->close();
    started_ = false;
  }

  template <typename T, typename U> T get(U &&request) {
    T result;
    std::stringstream stream;
    stream.write(reinterpret_cast<const char *>(request.data()),
                 request.size());
    {
      cereal::JSONInputArchive archive(stream);
      archive(result);
    }
    return std::move(result);
  }

  void recv();

  template <typename Response> void send(Response &&response) {

    // If response not set by user, set to empty JSON object
    if (!response.set_) {
      std::cout << "Response not set\n";
      std::string serialized = "{}"; // empty JSON
      response.payload_.rebuild(serialized.size());
      memcpy(response.payload_.data(), serialized.c_str(), serialized.size());
    }

    auto success = socket_->send(response.payload_, zmq::send_flags::none);
    while (!success) {
      socket_->send(response.payload_, zmq::send_flags::none);
    }
    ready_.notify_one();
  }

  void start();

  void stop() { 
    done_ = true; 
    ready_.notify_all();
  }
};

} // namespace internal

} // namespace iris