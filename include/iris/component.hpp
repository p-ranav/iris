#pragma once
#include <iris/task_system.hpp>
#include <iris/timer.hpp>
#include <iris/publisher.hpp>
#include <iris/subscriber.hpp>
#include <map>
#include <memory>
#include <zmq.hpp>

namespace iris {

  class component {
    task_system executor_;
    std::map<std::string, std::shared_ptr<timer>> timers_;
    std::map<std::string, std::shared_ptr<publisher>> publishers_;
    std::map<std::string, std::shared_ptr<subscriber>> subscribers_;        
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

    void add_publisher(std::string name, std::vector<std::string> endpoints) {
      auto p = std::make_shared<publisher>(context_,
					   std::move(endpoints),
					   executor_);
      publishers_.insert(std::make_pair(std::move(name), std::move(p)));
    }

    void add_subscriber(std::string name, std::vector<std::string> endpoints, std::string filter,
			std::function<void(std::string)> fn) {
      auto s = std::make_shared<subscriber>(context_,
					    std::move(endpoints),
					    std::move(filter),
					    operation::string_argument{.fn = fn},
					    executor_);
      subscribers_.insert(std::make_pair(std::move(name), std::move(s)));
    }    

    template <typename T>
    typename std::enable_if<std::is_same<T, timer>::value, std::weak_ptr<timer>>::type
    get(std::string name) {
      return timers_[std::move(name)];
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, publisher>::value, std::weak_ptr<publisher>>::type
    get(std::string name) {
      return publishers_[std::move(name)];
    }

    void start() {
      for (auto &[_, v] : subscribers_) {
	v->start();
      }            
      for (auto &[_, v] : timers_) {
	v->start();
      }
    }
  };

}
