#pragma once
#include <functional>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <zmq.hpp>

namespace iris {

class publisher {
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<task_system> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  std::vector<std::string> endpoints_;

public:
  publisher(zmq::context_t &context, std::vector<std::string> endpoints,
            task_system &executor)
      : context_(context), endpoints_(std::move(endpoints)),
        executor_(executor) {
    socket_ = std::make_unique<zmq::socket_t>(context_, ZMQ_PUB);
    for (auto &e : endpoints_)
      socket_->bind(e);
  }

  ~publisher() { socket_->close(); }

  template <typename Message>
  void send(Message &&message) {
    zmq::message_t message_struct(message.length());
    memcpy(message_struct.data(), message.c_str(), message.length());
    socket_->send(std::move(message_struct));
  }
};

} // namespace iris
