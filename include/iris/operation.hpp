#pragma once
#include <functional>
#include <string>
#include <variant>

namespace iris {

  namespace operation {

    struct void_argument {
      std::function<void()> fn;
    };
    
    struct string_argument {
      std::function<void(std::string)> fn;
      std::string arg;
    };

  }  
}
