#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <ratio>
#include <thread>
#include <iris/operation.hpp>
#include <iris/task_system.hpp>

namespace iris {

  class timer {
    std::chrono::milliseconds period_;
    operation::void_argument fn_;    
    std::reference_wrapper<task_system> executor_;
    
    std::atomic<bool> execute_{false};
    std::thread thread_;

  public:

    timer(unsigned int period, const operation::void_argument& fn, task_system &executor)
      : period_(std::chrono::milliseconds(period)), fn_(fn), executor_(executor),
	execute_(false), thread_({}) {}

    ~timer() {
      if(execute_.load(std::memory_order_acquire)) {
	stop();
      };
    }

    void stop() {
      execute_.store(false, std::memory_order_release);
      if(thread_.joinable())
	thread_.join();
    }

    void start() {      
      if(execute_.load(std::memory_order_acquire)) {
	stop();
      };
      execute_.store(true, std::memory_order_release);
      thread_ = std::thread([this]() {
			   while (execute_.load(std::memory_order_acquire)) {
			     executor_.get().async_(fn_);                   
			     std::this_thread::sleep_for(period_);
			   }
			 });
    }

    bool is_running() const noexcept {
      return (execute_.load(std::memory_order_acquire) && 
	      thread_.joinable());
    }

  };

}
