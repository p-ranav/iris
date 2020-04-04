#pragma once
#include <string>

namespace iris {

namespace internal {
class ServerImpl;
}

class Request {
  std::string payload_;
  class Component *component_;
  std::uint8_t server_id_;
  friend class TaskSystem;
  friend class internal::ServerImpl;

public:
  template <typename T> T get();
};

template <> inline std::string Request::get() { return payload_; }

}; // namespace iris