#pragma once
#include <string>
#include <iris/component.hpp>

namespace iris {

class subscriber_message {
    std::string payload_;
    class component * component_;
    std::uint8_t subscriber_id_;
    friend class zmq_subscriber;
public:
    template <typename T>
    T deserialize();
};

template <>
inline std::string subscriber_message::deserialize() {
    return payload_;
}

};