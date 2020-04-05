#pragma once
#include <iris/cereal/types/vector.hpp>
#include <string>
#include <vector>

struct Album {
  std::string name;
  std::string artist;
  int year;
  std::string genre;
  std::vector<std::string> tracks;

  template <class Archive> void serialize(Archive &ar) {
    ar(name, artist, year, genre);
    ar(tracks);
  }
};