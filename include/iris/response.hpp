#pragma once
#include <string>

namespace iris {

namespace internal {
class ClientImpl;
}

class Response {
  std::string payload_;
  class Component *component_;
  std::uint8_t client_id_;
  friend class internal::ClientImpl;

public:

  Response() : payload_(""), component_(nullptr), client_id_(0) {}

  template <typename T>
  Response(T && response) {
    // Serialize the response data
    std::stringstream stream;
    cereal::PortableBinaryOutputArchive archive(stream);
    archive(std::forward<T>(response));
    payload_ = stream.str();
  }

  template <typename Archive>
  void save(Archive& ar) const {
    ar(payload_);
  }

  template <typename Archive>
  void load(Archive& ar) {
    ar(payload_);
  }

  template <typename T> T get();
};

template <> inline Response::Response(std::string&& response) {
  payload_ = std::move(response);
}

template <> inline std::string Response::get() { return payload_; }

}; // namespace iris