#pragma once
#include <iris/cereal/types/vector.hpp>
#include <vector>

struct Numbers {
  std::vector<double> values;

  auto size() const { return values.size(); }
  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }

  template <class Archive> void serialize(Archive &ar) {
    ar(values);
  }
};

