#pragma once
#include <initializer_list>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/cppzmq/zmq.hpp>
#include <iris/internal/broker_impl.hpp>
#include <iris/internal/client_impl.hpp>
#include <iris/internal/oneshot_timer_impl.hpp>
#include <iris/internal/periodic_timer_impl.hpp>
#include <iris/internal/publisher_impl.hpp>
#include <iris/internal/server_impl.hpp>
#include <iris/internal/async_server_impl.hpp>
#include <iris/internal/subscriber_impl.hpp>
#include <iris/kwargs.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace iris {

class Component {
  friend TaskSystem;
  TaskSystem executor_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::PeriodicTimerImpl>>
      interval_timers_;
  std::vector<std::unique_ptr<internal::OneShotTimerImpl>> oneshot_timers_;
  std::vector<std::thread> oneshot_timer_threads_;

  std::unordered_map<std::uint8_t, std::unique_ptr<internal::PublisherImpl>>
      publishers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::SubscriberImpl>>
      subscribers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::ClientImpl>>
      clients_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::ServerImpl>>
      servers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::AsyncServerImpl>>
      async_servers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::BrokerImpl>>
      brokers_;
  zmq::context_t context_{zmq::context_t(1)};
  std::mutex timers_mutex_, oneshot_timers_mutex_, publishers_mutex_,
      subscribers_mutex_, clients_mutex_, servers_mutex_, async_servers_mutex_,
      brokers_mutex_;

  friend class PeriodicTimer;
  std::atomic_uint8_t timer_count_{0};
  void stop_timer(std::uint8_t timer_id) {
    lock_t lock{timers_mutex_};
    interval_timers_[timer_id]->stop();
  }

  friend class Publisher;
  std::atomic_uint8_t publisher_count_{0};
  template <typename Message>
  void publish(std::uint8_t publisher_id, Message &&message) {
    lock_t lock{publishers_mutex_};
    publishers_[publisher_id]->send(std::forward<Message>(message));
  }

  friend class Subscriber;
  std::atomic_uint8_t subscriber_count_{0};
  void stop_subscriber(std::uint8_t subscriber_id) {
    lock_t lock{subscribers_mutex_};
    subscribers_[subscriber_id]->stop();
  }

  friend class Client;
  std::atomic_uint8_t client_count_{0};
  template <typename R> Response request(std::uint8_t client_id, R &&request) {
    lock_t lock{clients_mutex_};
    return clients_[client_id]->send(std::forward<R>(request));
  }

  friend class Server;

  friend class Response;
  std::atomic_uint8_t server_count_{0};
  template <typename Response>
  void respond(std::uint8_t server_id, Response &&response) {
    lock_t lock{servers_mutex_};
    return servers_[server_id]->send(std::forward<Response>(response));
  }

  void stop_server(std::uint8_t server_id) {
    lock_t lock{servers_mutex_};
    servers_[server_id]->stop();
  }

  friend class AsyncServer;
  std::atomic_uint8_t async_server_count_{0};
  template <typename Response>
  void respond_async(std::uint8_t server_id, Response &&response) {
    lock_t lock{async_servers_mutex_};
    return async_servers_[server_id]->send(std::forward<Response>(response));
  }

  void stop_async_server(std::uint8_t server_id) {
    lock_t lock{async_servers_mutex_};
    async_servers_[server_id]->stop();
  }

  friend class Broker;
  std::atomic_uint8_t broker_count_{0};
  void stop_broker(std::uint8_t broker_id) {
    lock_t lock{brokers_mutex_};
    brokers_[broker_id]->stop();
  }

public:
  Component() : executor_(TaskSystem(std::thread::hardware_concurrency())) {}

  template <typename T>
  Component(T &&n) : executor_(TaskSystem(Threads(n).get())) {}

  ~Component() {
    for (auto &thread : executor_.threads_)
      if (thread.joinable())
        thread.join();
    subscribers_.clear();
    servers_.clear();
    async_servers_.clear();
    clients_.clear();
    publishers_.clear();
    interval_timers_.clear();
    brokers_.clear();
    for (auto &t : oneshot_timer_threads_) {
      if (t.joinable())
        t.join();
    }
    oneshot_timers_.clear();
  }

  template <typename P, typename T>
  class PeriodicTimer set_interval(P &&period_ms, T &&fn);

  template <typename P, typename T> void set_timeout(P &&delay_ms, T &&fn) {
    lock_t lock{oneshot_timers_mutex_};
    auto t = std::make_unique<internal::OneShotTimerImpl>(
        std::forward<DelayMs>(DelayMs(delay_ms)),
        operation::TimerOperation{.fn = TimerFunction(fn).get()}, executor_);
    oneshot_timers_.push_back(std::move(t));
  }

  template <typename E> class Publisher create_publisher(E &&endpoints);

  template <typename E, typename T, typename S>
  class Subscriber create_subscriber(E &&endpoints, T &&timeout, S &&fn);

  template <typename E, typename T, typename R>
  class Client create_client(E &&endpoints, T &&timeout, R &&retries);

  template <typename E, typename T, typename S>
  class Server create_server(E &&endpoints, T &&timeout, S &&fn);

  template <typename E, typename T, typename S>
  class AsyncServer create_async_server(E &&endpoints, T &&timeout, S &&fn);

  template <typename E>
  class Broker create_broker(E &&frontend_endpoints, E &&backend_endpoints);

  void join() {
    for (auto &thread : executor_.threads_)
      if (thread.joinable())
        thread.join();
  }

  void start() {
    executor_.start();
    for (auto &t : oneshot_timers_) {
      oneshot_timer_threads_.push_back(t->start());
    }
    for (auto &[_, v] : subscribers_) {
      if (v)
        v->start();
    }
    for (auto &[_, v] : servers_) {
      if (v)
        v->start();
    }
    for (auto &[_, v] : brokers_) {
      if (v)
        v->start();
    }
    for (auto &[_, v] : async_servers_) {
      if (v)
        v->start();
    }
    for (auto &[_, v] : interval_timers_) {
      if (v)
        v->start();
    }
  }

  void stop() {
    executor_.stop();
    for (auto &[_, v] : subscribers_) {
      if (v)
        v->stop();
    }
    for (auto &[_, v] : servers_) {
      if (v)
        v->stop();
    }
    for (auto &[_, v] : async_servers_) {
      if (v)
        v->stop();
    }
    for (auto &[_, v] : brokers_) {
      if (v)
        v->stop();
    }
    for (auto &[_, v] : interval_timers_) {
      if (v)
        v->stop();
    }
  }
};

void TaskSystem::run(unsigned i) {
  while (!done_) {
    lock_t lock{queue_mutex_};
    ready_.wait(lock);
    if (done_) break;
    operation_t op;
    for (unsigned n = 0; n != count_; ++n) {
      if (queue_[(i + n) % count_].try_pop(op))
        break;
    }
    if (!valid_operation(op) && !queue_[i].try_pop(op)) {
      continue;
    }
    if (auto void_op = std::get_if<operation::TimerOperation>(&op))
      (*void_op).fn();
    else if (auto subscriber_op =
                 std::get_if<operation::SubscriberOperation>(&op))
      (*subscriber_op).fn(std::move((*subscriber_op).arg));
    else if (auto server_op = std::get_if<operation::ServerOperation>(&op)) {
      Response response;
      auto request = (*server_op).arg;
      (*server_op).fn(request, response);
      // Send response back to client
      if (request.async_)
        request.component_->respond_async(request.server_id_, response);
      else
        request.component_->respond(request.server_id_, response);
    }
  }
}

} // namespace iris
