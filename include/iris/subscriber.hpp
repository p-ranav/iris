#pragma once
#include <atomic>
#include <zmq.hpp>
#include <functional>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <string>
#include <vector>
#include <queue>

namespace iris {

  class subscriber {
    std::reference_wrapper<zmq::context_t> context_;
    std::reference_wrapper<task_system> executor_;
    std::unique_ptr<zmq::socket_t> socket_;
    std::vector<std::string> endpoints_;
    std::string filter_;
    operation::string_argument fn_;
    std::thread thread_;
    std::atomic<bool> done_{false};

  public:
    subscriber(zmq::context_t &context, std::vector<std::string> endpoints,
	       std::string filter,
	       const operation::string_argument& fn, task_system &executor)
      : context_(context), endpoints_(std::move(endpoints)), filter_(std::move(filter)),
	fn_(fn), executor_(executor) {
      socket_ = std::make_unique<zmq::socket_t>(context, ZMQ_SUB);
      for (auto &e: endpoints_) {
	socket_->connect(e);
      }
      socket_->setsockopt(ZMQ_SUBSCRIBE, filter_.c_str(), filter_.length());
    }

    ~subscriber() {
      socket_->close();
    }

    void recv() {
      while(!done_) {
	zmq::message_t received_message;
	socket_->recv(&received_message);
	std::string message = std::string(static_cast<char*>(received_message.data()),
					  received_message.size());
	if (message.length() > 0) {
	  fn_.arg = std::move(message);
	  executor_.get().async_(fn_);	  
	}
      }
    }

    std::thread * start() {
      thread_ = std::thread(&subscriber::recv, this);
      return &thread_;
    }

    void stop() {
      done_ = true;
    }

  };

}
