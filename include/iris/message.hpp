#pragma once
#include <string>
#include <iris/component.hpp>

namespace iris {

class Message {
    std::string payload_;
    class Component * component_;
    std::uint8_t subscriber_id_;
    friend class zmq_subscriber;
public:
    template <typename T>
    T deserialize();
};

template <>
inline std::string Message::deserialize() {
    return payload_;
}

};