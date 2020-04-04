#pragma once
#include <atomic>
#include <functional>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/kwargs.hpp>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace iris {

namespace internal {

class SubscriberImpl {
  std::uint8_t id_;
  class Component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<TaskSystem> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;
  std::string filter_;
  operation::SubscriberOperation fn_;
  std::thread thread_;
  std::atomic_bool started_{false};
  std::atomic_bool done_{false};

  // Member variables for deserialization
  std::stringstream stream_;

public:
  SubscriberImpl(std::uint8_t id, Component *parent, zmq::context_t &context,
                 Endpoints endpoints, std::string filter, TimeoutMs timeout,
                 const operation::SubscriberOperation &fn,
                 TaskSystem &executor);

  ~SubscriberImpl() {
    if (started_)
      thread_.join();
    socket_->close();
    started_ = false;
  }

  template <typename T, typename U = std::string> T get(U &&message) {
    stream_ << message;
    cereal::PortableBinaryInputArchive archive(stream_);
    T result;
    archive(result);
    return std::move(result);
  }

  void recv();

  void start();

  void stop() { done_ = true; }
};

} // namespace internal

} // namespace iris
