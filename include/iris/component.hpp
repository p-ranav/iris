#pragma once
#include <iris/task_system.hpp>
#include <iris/timer.hpp>
#include <map>
#include <memory>
#include <zmq.hpp>

namespace iris {

  class component {
    task_system executor_;
    std::map<std::string, std::shared_ptr<timer>> timers_;
    zmq::context_t context_{zmq::context_t(1)};

  public:

    ~component() {
      for (auto &queue : executor_.queue_)
	queue.done();
      for (auto &thread : executor_.threads_)
	thread.join();      
    }
    
    void add_timer(std::string name, unsigned int period, std::function<void()> fn) {
      auto t = std::make_shared<timer>(period,
				       operation::void_argument{.fn = fn},
				       executor_);
      timers_.insert(std::make_pair(std::move(name), std::move(t)));
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, timer>::value, std::weak_ptr<timer>>::type
    get(std::string name) {
      return timers_[std::move(name)];
    }

    void start() {
      for (auto &[_, v] : timers_) {
	v->start();
      }
    }
  };

}
