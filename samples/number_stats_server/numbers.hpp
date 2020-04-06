#pragma once
#include <iris/cereal/types/array.hpp>
#include <array>

struct Numbers {
  std::array<double, 3> values;

  auto size() const { return values.size(); }
  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }

  template <class Archive> void serialize(Archive &ar) {
    ar(values);
  }
};

