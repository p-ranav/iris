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

using DelayMs = fluent::NamedType<unsigned int, struct DelayMsTag>;
namespace iris {
static const DelayMs::argument delay;
}

using TimerFunction =
    fluent::NamedType<std::function<void()>, struct TimerFunctionTag>;
namespace iris {
static const TimerFunction::argument on_triggered;
}

using Endpoints = std::vector<std::string>;
namespace iris {
static Endpoints endpoints;
static Endpoints frontend;
static Endpoints backend;
}

using TimeoutMs = fluent::NamedType<int, struct TimeoutMsTag>;
namespace iris {
static const TimeoutMs::argument timeout;
}

using Retries = fluent::NamedType<unsigned int, struct RetriesTag>;
namespace iris {
static const Retries::argument retries;
}

using SubscriberFunction = fluent::NamedType<std::function<void(iris::Message)>,
                                             struct SubscriberFunctionTag>;
namespace iris {
static const SubscriberFunction::argument on_receive;
}

using ServerFunction =
    fluent::NamedType<std::function<void(iris::Request, iris::Response &)>,
                      struct ServerFunctionTag>;
namespace iris {
static const ServerFunction::argument on_request;
}