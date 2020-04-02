#pragma once
#include <functional>
#include <iris/named_type/named_type.hpp>
#include <iris/message.hpp>

using Threads = fluent::NamedType<unsigned, struct ThreadsTag>;
static const Threads::argument threads;

using PeriodMs = fluent::NamedType<unsigned int, struct PeriodMsTag>;
static const PeriodMs::argument period;

using TimerFunction =
    fluent::NamedType<std::function<void()>, struct TimerFunctionTag>;
static const TimerFunction::argument on_expiry;

using Endpoints = std::vector<std::string>;
static Endpoints endpoints;

using SubscriberFunction = fluent::NamedType<std::function<void(iris::Message)>,
                                             struct SubscriberFunctionTag>;
static const SubscriberFunction::argument on_receive;