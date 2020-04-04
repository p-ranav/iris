#pragma once
#include <functional>
#include <iostream>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/cereal/archives/json.hpp>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <iris/cppzmq/zmq.hpp>

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

  template <typename Message> Response send(Message &&message) {
    std::stringstream stream;
    cereal::JSONOutputArchive archive(stream);
    archive(message);
    send(stream.str().c_str());
  }

  Response send(std::string message) { send(message.c_str()); }

  struct Album {
    std::string name;
    std::string artist;
    int year;
    std::string genre;
    std::vector<std::string> tracks;

    template <class Archive>
    void serialize( Archive & ar ) {
      ar(year);
    }

};

  Response send(const char *message) {
    zmq::message_t message_struct(strlen(message));
    memcpy(message_struct.data(), message, strlen(message));

    unsigned retries_left = retries_.get();
    while (retries_left) {
      socket_->send(std::move(message_struct));
      bool expect_reply = true;

      while (expect_reply) {
        //  Poll socket for a reply, with timeout
        zmq::pollitem_t items[] = {
            {static_cast<void *>(*socket_), 0, ZMQ_POLLIN, 0}};
        zmq::poll(&items[0], 1, timeout_.get());

        //  If we got a reply, process it
        if (items[0].revents & ZMQ_POLLIN) {
          zmq::message_t reply;
          socket_->recv(reply);
          const auto response =
              std::string(static_cast<char *>(reply.data()), reply.size());

          std::stringstream test_stream;
          test_stream << response;
          cereal::JSONInputArchive test_archive(test_stream);
          Album test_result;
          test_archive(test_result);
          std::cout << test_result.name << std::endl;
          std::cout << test_result.year << std::endl;

          Response result;
          result.payload_ = std::string(static_cast<char *>(reply.data()), reply.size());
          result.client_id_ = id_;
          result.component_ = component_;
          return std::move(result);
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
          zmq::message_t message_struct(strlen(message));
          memcpy(message_struct.data(), message, strlen(message));
          socket_->send(std::move(message_struct));
        }
      }
    }

    // Wait for response
    // Deserialize as Response type and return to client
    zmq::message_t reply;
    socket_->recv(reply);
    const auto response =
        std::string(static_cast<char *>(reply.data()), reply.size());
    Response result;
    result.payload_ = response;
    result.client_id_ = id_;
    result.component_ = component_;
    return std::move(result);
  }
};

} // namespace internal

} // namespace iris
