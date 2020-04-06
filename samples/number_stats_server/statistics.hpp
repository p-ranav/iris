#pragma once

struct Statistics {
  double mean;
  double stdev;

  template <class Archive> void serialize(Archive &ar) {
    ar(mean, stdev);
  }
};

