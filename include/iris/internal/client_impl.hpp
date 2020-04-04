#pragma once
#include <functional>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <zmq.hpp>

namespace iris {

namespace internal {

class ClientImpl {
  std::uint8_t id_;
  class Component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<TaskSystem> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;

public:
  ClientImpl(zmq::context_t &context, Endpoints endpoints,
                TimeoutMs timeout,
                TaskSystem &executor)
      : context_(context), endpoints_(std::move(endpoints)),
        executor_(executor) {
    socket_ = std::make_unique<zmq::socket_t>(context_, ZMQ_REQ);
    for (auto &e : endpoints_)
      socket_->connect(e);
    socket_->set(zmq::sockopt::rcvtimeo, timeout.get());
  }

  ~ClientImpl() { socket_->close(); }

  template <typename Message> Response send(Message &&message) {
    std::stringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(message);
    auto message_str = stream.str();

    zmq::message_t message_struct(message_str.length());
    memcpy(message_struct.data(), message_str.c_str(), message_str.length());
    socket_->send(std::move(message_struct));

    // Wait for response
    // Deserialize as Response type and return to client
    zmq::message_t reply;
    socket_->recv(&reply);
    const auto response = std::string(static_cast<char*>(reply.data()), reply.size());
    Response result;
    result.payload_ = response;
    result.client_id_ = id_;
    result.component_ = component_;
    return std::move(result);
  }

  Response send(std::string message) { send(message.c_str()); }

  Response send(const char *message) {
    zmq::message_t message_struct(strlen(message));
    memcpy(message_struct.data(), message, strlen(message));
    socket_->send(std::move(message_struct));

    // Wait for response
    // Deserialize as Response type and return to client
    zmq::message_t reply;
    socket_->recv(reply);
    const auto response = std::string(static_cast<char*>(reply.data()), reply.size());
    Response result;
    result.payload_ = response;
    result.client_id_ = id_;
    result.component_ = component_;
    return std::move(result);
  }
};

} // namespace internal

} // namespace iris