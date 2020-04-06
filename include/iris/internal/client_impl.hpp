#pragma once
#include <functional>
#include <iostream>
#include <iris/cereal/archives/json.hpp>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/cppzmq/zmq.hpp>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/response.hpp>
#include <iris/task_system.hpp>
#include <memory>

namespace iris {

namespace internal {

class ClientImpl {
  std::uint8_t id_;
  class Component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<TaskSystem> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;
  TimeoutMs timeout_;
  Retries retries_;

public:
  template <typename E, typename T, typename R>
  ClientImpl(zmq::context_t &context, E &&endpoints, T &&timeout, R &&retries,
             TaskSystem &executor)
      : context_(context), endpoints_(std::move(endpoints)), timeout_(timeout),
        retries_(retries), executor_(executor) {
    socket_ = std::make_unique<zmq::socket_t>(context_, ZMQ_REQ);
    for (auto &e : endpoints_)
      socket_->connect(e);
    socket_->set(zmq::sockopt::rcvtimeo, timeout_.get());
    int linger = 0;
    socket_->set(zmq::sockopt::linger, linger);
  }

  ~ClientImpl() { socket_->close(); }

  Response send(const char *request) { return send(std::string(request)); }

  template <typename R> Response send(R &&request) {
    std::stringstream stream;
    {
      cereal::JSONOutputArchive archive(
          stream, cereal::JSONOutputArchive::Options::NoIndent());
      archive(request);
    }
    auto serialized = stream.str();
    zmq::message_t message_struct(serialized.size());
    memcpy(message_struct.data(), serialized.c_str(), serialized.size());

    unsigned retries_left = retries_.get();
    while (retries_left) {
      socket_->send(std::move(message_struct), zmq::send_flags::none);
      bool expect_reply = true;

      while (expect_reply) {
        //  Poll socket for a reply, with timeout
        zmq::pollitem_t items[] = {
            {static_cast<void *>(*socket_), 0, ZMQ_POLLIN, 0}};
        zmq::poll(&items[0], 1, timeout_.get());

        //  If we got a reply, process it
        if (items[0].revents & ZMQ_POLLIN) {
          zmq::message_t reply;
          auto ret = socket_->recv(reply);
          if (ret.has_value()) {
            Response result;
            result.payload_ = std::move(reply);
            result.client_id_ = id_;
            result.component_ = component_;
            return std::move(result);
          }
          // TODO: Handle coming here
        } else if (--retries_left == 0) {
          // std::cout << "E: server seems to be offline, abandoning" <<
          // std::endl;
          expect_reply = false;
          break;
        } else {
          // std::cout << "W: no response from server, retrying…" << std::endl;
          //  Old socket will be confused; close it and open a new one
          socket_->close();
          socket_.reset(new zmq::socket_t(context_, ZMQ_REQ));
          for (auto &e : endpoints_)
            socket_->connect(e);
          socket_->set(zmq::sockopt::rcvtimeo, timeout_.get());
          int linger = 0;
          socket_->set(zmq::sockopt::linger, linger);

          //  Send request again, on new socket
          zmq::message_t message_struct(serialized.size());
          memcpy(message_struct.data(), serialized.c_str(), serialized.size());
          socket_->send(std::move(message_struct), zmq::send_flags::none);
        }
      }
    }

    // TODO: Check if server is down and throw to calling business logic

    // Wait for response
    // Deserialize as Response type and return to client
    zmq::message_t reply;
    auto ret = socket_->recv(reply);
    Response result;
    if (ret) {
      result.payload_ = std::move(reply);
      result.client_id_ = id_;
      result.component_ = component_;
      return std::move(result);
    } else {
      // TODO Handle this better
      return result;
    }
  }
};

} // namespace internal

} // namespace iris
