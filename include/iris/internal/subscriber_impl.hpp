#pragma once
#include <atomic>
#include <functional>
#include <iris/cereal/archives/json.hpp>
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

class SubscriberImpl {
  std::uint8_t id_;
  class Component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<TaskSystem> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;
  operation::SubscriberOperation fn_;
  std::thread thread_;
  std::atomic_bool started_{false};
  std::atomic_bool done_{false};

public:
  template <typename E, typename T, typename S>
  SubscriberImpl(std::uint8_t id, Component *parent, zmq::context_t &context,
                 E &&endpoints, T &&timeout, S &&fn,
                 TaskSystem &executor);

  ~SubscriberImpl() {
    if (started_)
      if (thread_.joinable())
        thread_.join();
    socket_->close();
    started_ = false;
  }

  template <typename T, typename U> T get(U &&message) {
    T result;
    std::stringstream stream;
    stream.write(reinterpret_cast<const char *>(message.data()),
                 message.size());
    {
      cereal::JSONInputArchive archive(stream);
      archive(result);
    }
    return std::move(result);
  }

  void recv();

  void start();

  void stop() { done_ = true; }
};

} // namespace internal

} // namespace iris
