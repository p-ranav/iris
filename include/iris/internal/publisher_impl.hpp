#pragma once
#include <functional>
#include <iris/cereal/archives/json.hpp>
#include <iris/cppzmq/zmq.hpp>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>

namespace iris {

namespace internal {

class PublisherImpl {
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<TaskSystem> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;

public:
  template <typename E>
  PublisherImpl(zmq::context_t &context, E &&endpoints, TaskSystem &executor)
      : context_(context), endpoints_(std::move(endpoints)),
        executor_(executor) {
    socket_ = std::make_unique<zmq::socket_t>(context_, ZMQ_PUB);
    for (auto &e : endpoints_)
      socket_->bind(e);
    socket_->set(zmq::sockopt::sndtimeo, 0);
  }

  ~PublisherImpl() { socket_->close(); }

  template <typename M> void send(M &&message) {
    std::stringstream stream;
    {
      cereal::JSONOutputArchive archive(
          stream, cereal::JSONOutputArchive::Options::NoIndent());
      archive(message);
    }
    auto message_str = stream.str();
    zmq::message_t message_struct(message_str.length());
    memcpy(message_struct.data(), message_str.c_str(), message_str.length());
    socket_->send(std::move(message_struct));
  }
};

} // namespace internal

} // namespace iris
