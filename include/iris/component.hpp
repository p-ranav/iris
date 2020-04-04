#pragma once
#include <initializer_list>
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/internal/client_impl.hpp>
#include <iris/internal/periodic_timer_impl.hpp>
#include <iris/internal/publisher_impl.hpp>
#include <iris/internal/server_impl.hpp>
#include <iris/internal/subscriber_impl.hpp>
#include <iris/kwargs.hpp>
#include <iris/task_system.hpp>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

namespace iris {

class Component {
  friend TaskSystem;
  TaskSystem executor_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::PeriodicTimerImpl>>
      interval_timers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::PublisherImpl>>
      publishers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::SubscriberImpl>>
      subscribers_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::ClientImpl>>
      clients_;
  std::unordered_map<std::uint8_t, std::unique_ptr<internal::ServerImpl>>
      servers_;
  zmq::context_t context_{zmq::context_t(1)};
  std::mutex timers_mutex_, publishers_mutex_, subscribers_mutex_,
      clients_mutex_, servers_mutex_;

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

  friend class Message;
  template <typename T, typename U = std::string>
  T get(std::uint8_t subscriber_id, U &&message) {
    lock_t lock{subscribers_mutex_};
    return subscribers_[subscriber_id]->get<T>(std::forward<U>(message));
  }

  friend class Client;
  std::atomic_uint8_t client_count_{0};
  template <typename Request>
  Response request(std::uint8_t client_id, Request &&request) {
    lock_t lock{clients_mutex_};
    return clients_[client_id]->send(std::forward<Request>(request));
  }

  friend class Server;
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

public:
  Component() : executor_(TaskSystem(std::thread::hardware_concurrency())) {}

  template <typename T>
  Component(T &&n) : executor_(TaskSystem(Threads(n).get())) {}

  ~Component() {
    for (auto &thread : executor_.threads_)
      thread.join();
    subscribers_.clear();
    publishers_.clear();
    interval_timers_.clear();
  }

  template <typename P, typename T>
  class PeriodicTimer set_interval(P &&period_ms, T &&fn);

  template <typename E> class Publisher create_publisher(E &&endpoints);

  template <typename E, typename T, typename S>
  class Subscriber create_subscriber(E &&endpoints, T &&timeout, S &&fn);

  template <typename E, typename T>
  class Client create_client(E &&endpoints, T &&timeout);

  template <typename E, typename T, typename S>
  class Server create_server(E &&endpoints, T &&timeout, S &&fn);

  void start() {
    executor_.start();
    for (auto &[_, v] : subscribers_) {
      v->start();
    }
    for (auto &[_, v] : servers_) {
      v->start();
    }
    for (auto &[_, v] : interval_timers_) {
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
    for (auto &[_, v] : interval_timers_) {
      if (v)
        v->stop();
    }
  }
};

void TaskSystem::run(unsigned i) {
  while (!done_) {
    operation_t op;
    for (unsigned n = 0; n != count_; ++n) {
      if (queue_[(i + n) % count_].try_pop(op))
        break;
    }
    if (!valid_operation(op) && !queue_[i].try_pop(op))
      continue;
    if (auto void_op = std::get_if<operation::TimerOperation>(&op))
      (*void_op).fn();
    else if (auto subscriber_op =
                 std::get_if<operation::SubscriberOperation>(&op))
      (*subscriber_op).fn(std::move((*subscriber_op).arg));
    else if (auto server_op = std::get_if<operation::ServerOperation>(&op)) {
      auto request = (*server_op).arg;
      auto response = (*server_op).fn(request);
      // Send response back to client
      request.component_->respond(request.server_id_, std::move(response));
    }
  }
}

} // namespace iris
