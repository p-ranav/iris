#pragma once
#include <string>

struct NginxLogEntry {
  std::string time;
  std::string remote_ip;
  std::string remote_user;
  std::string request;
  unsigned response;
  unsigned bytes;
  std::string agent;

  template <class Archive> void serialize(Archive &ar) {
    ar(time, remote_ip, remote_user, request, response, bytes, agent);
  }
};