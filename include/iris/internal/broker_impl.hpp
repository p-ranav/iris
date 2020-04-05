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

class BrokerImpl {
  std::uint8_t id_;
  class Component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  
  std::unique_ptr<zmq::socket_t> frontend_;
  Endpoints frontend_endpoints_;

  std::unique_ptr<zmq::socket_t> backend_;
  Endpoints backend_endpoints_;

  std::thread thread_;
  std::atomic_bool started_{false};
  std::atomic_bool done_{false};
  std::atomic_bool ready_{false};

public:
  template <typename E>
  BrokerImpl(std::uint8_t id, Component *parent, zmq::context_t &context,
             E &&frontend_endpoints, E &&backend_endpoints) 
             : id_(id), component_(parent), context_(context),
             frontend_endpoints_(std::move(frontend_endpoints)),
             backend_endpoints_(std::move(backend_endpoints)) {
    frontend_ = std::make_unique<zmq::socket_t>(context_, ZMQ_ROUTER);
    backend_ = std::make_unique<zmq::socket_t>(context_, ZMQ_DEALER);
    for (auto &e : frontend_endpoints_) {
        frontend_->bind(e);
    }
    for (auto &e : backend_endpoints_) {
        backend_->bind(e);
    }
  }

  ~BrokerImpl() {
    if (started_)
      thread_.join();
    frontend_->close();
    backend_->close();
    started_ = false;
  }

  void recv() {
    //  Initialize poll set
    zmq::pollitem_t items [] = {
        { static_cast<void*>(*frontend_.get()), 0, ZMQ_POLLIN, 0 },
        { static_cast<void*>(*backend_.get()), 0, ZMQ_POLLIN, 0 }
    };

    std::cout << "Starting recv\n";
    
    //  Switch messages between sockets
    while (!done_) {
        zmq::message_t message;
        int more;               //  Multipart detection

        zmq::poll(&items[0], 2, -1);

        std::cout << "created poll\n";
        
        if (items[0].revents & ZMQ_POLLIN) {
            while(!done_) {
                //  Process all parts of the message
                frontend_->recv(&message);
                size_t more_size = sizeof (more);
                frontend_->getsockopt(ZMQ_RCVMORE, &more, &more_size);
                backend_->send(message, more? ZMQ_SNDMORE: 0);
                
                if (!more)
                    break;      //  Last message part
            }
        }
        if (items[1].revents & ZMQ_POLLIN) {
            while (!done_) {
                //  Process all parts of the message
                backend_->recv(&message);
                size_t more_size = sizeof (more);
                backend_->getsockopt(ZMQ_RCVMORE, &more, &more_size);
                frontend_->send(message, more? ZMQ_SNDMORE: 0);
                if (!more)
                    break;      //  Last message part
            }
        }
    }
  }

  void start() {
    thread_ = std::thread(&BrokerImpl::recv, this);
    std::cout << "Started\n";
    started_ = true;
    ready_ = true;
  }

  void stop() { done_ = true; }
};

} // namespace internal

} // namespace iris