#include "album.hpp"
#include <iris/iris.hpp>
using namespace iris;
#include <iostream>
#include <map>

int main() {

  std::map<std::string, Album> albums;
  albums["R2 552927"] =
      Album{.name = "Paranoid",
            .artist = "Black Sabbath",
            .year = 1970,
            .genre = "Heavy metal",
            .tracks = {"War Pigs", "Paranoid", "Planet Caravan", "Iron Man",
                       "Electric Funeral", "Hand of Doom",
                       "Jack the Stripper / Fairies Wear Boots"}};

  Component music_tag_component;
  music_tag_component.create_server(
      endpoints = {"tcp://*:5510"}, timeout = 500,
      on_request = [&](Request request, Response& response) {
          auto catalog_id = request.get<std::string>();
          std::cout << "Received request for catalog # " << catalog_id << "\n";
          response.set(albums[catalog_id]);
      });
  music_tag_component.start();
}