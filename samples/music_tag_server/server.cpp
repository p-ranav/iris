#include "album.hpp"
#include <iris/iris.hpp>
using namespace iris;
#include <iostream>
#include <map>
#include "json.hpp"
#include <fstream>
#include <iris/cereal/types/optional.hpp>
#include <iris/cereal/types/tuple.hpp>

int main() {

  // Load JSON database
  nlohmann::json j;
  std::ifstream stream("database.json");
  stream >> j;

  Component server;
  server.create_server(
      endpoints = {"tcp://*:5510"}, 
      timeout = 500,
      on_request = [&](Request request, Response &response) {
          // Request from client
          auto kvpair = request.get<std::tuple<std::string, std::string>>();
          auto key = std::get<0>(kvpair);
          auto value = std::get<1>(kvpair);
          std::cout << "Received request {key: " 
                    << key << ", value: " << value << "}\n";

          // Response to be filled and sent back 
          // Either a valid album struct or empty
          std::optional<Album> album{};

          // Find the album in the JSON database
          auto it = std::find_if(j.begin(), j.end(), 
            [&key, &value](const auto& element) {
              if (key == "year")
                return element[key] == std::stoi(value);
              else
                return element[key] == value;
          });

          // Populate the response fields
          if (it != j.end()) {
            album = Album {
              .name = (*it)["name"].get<std::string>(),
              .artist = (*it)["artist"].get<std::string>(),
              .year = (*it)["year"].get<unsigned>(),
              .genre = (*it)["genre"].get<std::string>(),
              .tracks = (*it)["tracks"].get<std::vector<std::string>>()
            };
          }

          // Set response
          // response.set(album);
      });
  server.start();
}