#pragma once
#include <iris/named_type.hpp>
#include <functional>

using PeriodMs = fluent::NamedType<unsigned int, struct PeriodMsTag>;
static const PeriodMs::argument period;

using TimerFunction = fluent::NamedType<std::function<void()>, struct TimerFunctionTag>;
static const TimerFunction::argument on_expiry;