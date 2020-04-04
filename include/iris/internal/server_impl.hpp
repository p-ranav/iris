#pragma once
#include <atomic>
#include <functional>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/cereal/archives/json.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <iris/kwargs.hpp>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <zmq.hpp>

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
  std::atomic_bool ready_{false};

public:
  ServerImpl(std::uint8_t id, Component *parent, zmq::context_t &context,
                 Endpoints endpoints,
                 TimeoutMs timeout,
                 const operation::ServerOperation &fn,
                 TaskSystem &executor);

  ~ServerImpl() {
    if (started_)
      thread_.join();
    socket_->close();
    started_ = false;
  }

  template <typename T, typename U = std::string> T get(U &&request) {
    std::stringstream stream;
    stream << request;
    cereal::PortableBinaryInputArchive archive(stream);
    T result;
    archive(result);
    return std::move(result);
  }

  void recv();

  template <typename Response> void send(Response &&response) {
    std::cout << "Sending response back to client\n";
    std::cout << response.payload_ << std::endl;
    // std::stringstream stream;
    // cereal::JSONOutputArchive archive(stream);
    // archive(response);
    const auto response_str = response.payload_; // stream.str();
    std::cout << "Serialized response: " << response_str << std::endl;
    zmq::message_t reply(response_str.length());
    memcpy(reply.data(), response_str.c_str(), response_str.length());
    socket_->send(reply);
    ready_ = true;
  }

  void start();

  void stop() { done_ = true; }
};

} // namespace internal

} // namespace iris