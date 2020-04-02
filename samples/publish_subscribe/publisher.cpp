#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>
#include <iris/timer.hpp>
#include <sstream>
#include <iris/cereal/archives/json.hpp>

struct Foo {
  int value_{15};
  template <class Archive>
  void save(Archive & ar) const {
    ar(value_);
  }
      
  template <class Archive>
  void load(Archive & ar) {
    ar(value_);
  }
};

int main() {

  std::stringstream stream;
  {
    cereal::JSONOutputArchive archive(stream);
    Foo bar;
    bar.value_ = 10;
    archive(bar);
    std::cout << stream.str() << std::endl;
  }

  {
    std::stringstream test;
    test << "{ \"value0\": { \"value0\": 10 } }"; 
    Foo baz;
    cereal::JSONInputArchive input_archive(test);
    input_archive(baz);
    std::cout << baz.value_ << std::endl;
  }

}