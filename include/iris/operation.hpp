#pragma once
#include <functional>
#include <string>
#include <variant>

namespace iris {

  namespace operation {

    struct no_argument {
      std::function<void()> fn;
    };
    
    struct string_argument {
      std::function<void(std::string)> fn;
      std::string arg;
    }

  }

  using operation_t = std::variant<no_argument, string_argument>;
}
