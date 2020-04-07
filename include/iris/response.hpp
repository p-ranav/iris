#pragma once
#include <iris/cereal/archives/portable_binary.hpp>
#include <iris/cppzmq/zmq.hpp>
#include <sstream>

namespace iris {

namespace internal {
class ClientImpl;
}

class Response {
  zmq::message_t payload_;
  class Component *component_;
  std::uint8_t client_id_;
  bool set_{false};

  friend class internal::ClientImpl;
  friend class internal::ServerImpl;
  friend class internal::AsyncServerImpl;

public:
  Response() {}

  Response(const Response &rhs) {
    payload_.copy(const_cast<Response &>(rhs).payload_);
    component_ = rhs.component_;
    client_id_ = rhs.client_id_;
    set_ = rhs.set_;
  }

  Response &operator=(Response rhs) {
    std::swap(payload_, rhs.payload_);
    std::swap(component_, rhs.component_);
    std::swap(client_id_, rhs.client_id_);
    std::swap(set_, rhs.set_);
    return *this;
  }

  template <typename T> T get() {
    T result;
    std::stringstream stream;
    stream.write(reinterpret_cast<const char *>(payload_.data()),
                 payload_.size());
    {
      cereal::JSONInputArchive archive(stream);
      archive(result);
    }
    return std::move(result);
  }

  template <typename T> void set(T &&response) {
    // Serialize the response data
    std::stringstream stream;
    {
      cereal::JSONOutputArchive archive(
          stream, cereal::JSONOutputArchive::Options::NoIndent());
      archive(response);
    }
    auto serialized = stream.str();
    payload_.rebuild(serialized.size());
    memcpy(payload_.data(), serialized.c_str(), serialized.size());
    set_ = true;
  }

  bool has_value() const { return set_; }
};

}; // namespace iris