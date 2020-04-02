#pragma once
#include <iris/named_type.hpp>

using PeriodMs = fluent::NamedType<unsigned int, struct PeriodMsTag>;
static const PeriodMs::argument period;