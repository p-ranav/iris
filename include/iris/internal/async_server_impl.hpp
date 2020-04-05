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

class AsyncServerImpl {
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
  std::atomic_bool ready_{false};

public:
  template <typename E, typename T, typename S>
  AsyncServerImpl(std::uint8_t id, Component *parent, zmq::context_t &context,
             E &&endpoints, T &&timeout, S &&fn, TaskSystem &executor);

  ~AsyncServerImpl() {
    if (started_)
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
    std::cout << "Scket Level:: Sending server response\n";
    auto success = socket_->send(response.payload_);
    while (!success) {
      socket_->send(response.payload_);
    }
    ready_ = true;
  }

  void start();

  void stop() { done_ = true; }
};

} // namespace internal

} // namespace iris