#include "album.hpp"
#include <iostream>
#include <optional>
#include <iris/iris.hpp>
using namespace iris;
#include <iris/cereal/types/optional.hpp>
#include <iris/cereal/types/tuple.hpp>

int main(int argc, char *argv[]) {

  std::tuple<std::string, std::string> request;

  if (argc != 3) {
    std::cout << "Usage: ./<executable> key value\n";
    return 0;
  } else {
    request = {argv[1], argv[2]};
  }

  Component c(threads = 1);
  c.start();
  auto client = c.create_client(endpoints = {"tcp://127.0.0.1:5510"},
                                timeout = 2500, retries = 3);

  // Send request to server
  auto response = client.send(request);

  // Parse response and print result if available
  auto album = response.get<std::optional<Album>>();
  if (album.has_value()) {
    auto metadata = album.value();
    std::cout << "- Received album:\n";
    std::cout << "    Name: " << metadata.name << "\n";
    std::cout << "    Artist: " << metadata.artist << "\n";
    std::cout << "    Year: " << metadata.year << "\n";
    std::cout << "    Genre: " << metadata.genre << "\n";
    std::cout << "    Tracks:\n";
    for (size_t i = 0; i < metadata.tracks.size(); ++i) {
      std::cout << "      " << i << ". " << metadata.tracks[i] << "\n";
    }
  } else {
    std::cout << "Album not found!\n";
  }
  c.stop();
}