#pragma once
#include <iris/task_system.hpp>
#include <iris/timer.hpp>
#include <vector>
#include <memory>

namespace iris {

  class component {
    task_system executor_;
    std::vector<std::unique_ptr<timer>> timers_;

  public:

    ~component() {
      for (auto &queue : executor_.queue_)
	queue.done();
      for (auto &thread : executor_.threads_)
	thread.join();      
      std::cout << "Component destructed\n";
    }
    
    void add_timer(long long period, std::function<void()> fn) {
      std::cout << "Adding timer\n";
      auto t = std::make_unique<timer>(period,
				       operation::void_argument{.fn = fn},
				       executor_);
      timers_.push_back(std::move(t));
    }

    void start() {
      for (auto &t : timers_) {
	std::cout << "Calling timer start\n";
	t->start();
      }
    }
  };

}
