#pragma once
#include <string>
#include <vector>

struct Album {
  std::string name;
  std::string artist;
  int year;
  std::string genre;
  std::vector<std::string> tracks;

  template <class Archive>
  void serialize( Archive & ar ) {
    // ar(name);
    // ar(artist);
    ar(year);
  }

};