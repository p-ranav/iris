#pragma once
#include <atomic>
#include <functional>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <iris/cereal/archives/portable_binary.hpp>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace iris {

class zmq_subscriber {
  std::uint8_t id_;
  class component *component_;
  std::reference_wrapper<zmq::context_t> context_;
  std::reference_wrapper<task_system> executor_;
  std::unique_ptr<zmq::socket_t> socket_;
  Endpoints endpoints_;
  std::string filter_;
  operation::subscriber_operation fn_;
  std::thread thread_;
  std::atomic_bool started_{false};
  std::atomic_bool done_{false};

  // Member variables for deserialization
  std::stringstream stream_;

public:
  zmq_subscriber(std::uint8_t id, component * parent, zmq::context_t &context, Endpoints endpoints,
                 std::string filter, const operation::subscriber_operation &fn,
                 task_system &executor);

  ~zmq_subscriber() {
    if (started_)
      thread_.join();
    socket_->close();
    started_ = false;
  }

  template <typename T, typename U = std::string>
  T deserialize(U&& message) {
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

} // namespace iris
