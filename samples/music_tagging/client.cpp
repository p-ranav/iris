#include "album.hpp"
#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component c(threads = 1);
  auto client = c.create_client(endpoints = {"tcp://127.0.0.1:5510"},
                                timeout = 2500, retries = 3);

  c.start();

  std::string request = "R2 552927";
  std::cout << "Sending request with catalog# " << request << std::endl;
  auto response = client.send(request);

  auto album = response.get<Album>();
  std::cout << "- Received album:\n";
  std::cout << "    Name: " << album.name << "\n";
  std::cout << "    Artist: " << album.artist << "\n";
  std::cout << "    Year: " << album.year << "\n";
  std::cout << "    Genre: " << album.genre << "\n";
  std::cout << "    Tracks:\n";
  for (size_t i = 0; i < album.tracks.size(); ++i) {
    std::cout << "      " << 
              i << ". " << album.tracks[i] << "\n";
  }
  std::cout << std::endl;
  
  c.stop();
}