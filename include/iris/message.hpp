#pragma once
#include <string>

namespace iris {

namespace internal {
class SubscriberImpl;
}

class Message {
  std::string payload_;
  class Component *component_;
  std::uint8_t subscriber_id_;
  friend class internal::SubscriberImpl;

public:
  template <typename T> T get();
};

template <> inline std::string Message::get() { return payload_; }

}; // namespace iris