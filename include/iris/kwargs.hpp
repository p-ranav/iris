#pragma once
#include <functional>
#include <iris/message.hpp>
#include <iris/named_type/named_type.hpp>
#include <iris/request.hpp>
#include <iris/response.hpp>

using Threads = fluent::NamedType<unsigned, struct ThreadsTag>;
namespace iris {
static const Threads::argument threads;
}

using PeriodMs = fluent::NamedType<unsigned int, struct PeriodMsTag>;
namespace iris {
static const PeriodMs::argument period;
}

using TimerFunction =
    fluent::NamedType<std::function<void()>, struct TimerFunctionTag>;
namespace iris {
static const TimerFunction::argument on_expiry;
}

using Endpoints = std::vector<std::string>;
namespace iris {
static Endpoints endpoints;
}

using TimeoutMs = fluent::NamedType<int, struct TimeoutMsTag>;
namespace iris {
static const TimeoutMs::argument timeout;
}

using SubscriberFunction = fluent::NamedType<std::function<void(iris::Message)>,
                                             struct SubscriberFunctionTag>;
namespace iris {
static const SubscriberFunction::argument on_receive;
}

using ServerFunction =
    fluent::NamedType<std::function<iris::Response(iris::Request)>,
                      struct ServerFunctionTag>;
namespace iris {
static const ServerFunction::argument on_request;
}