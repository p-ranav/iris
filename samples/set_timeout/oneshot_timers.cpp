#include <iris/iris.hpp>
using namespace iris;
#include <iostream>

int main() {
    Component c;
    c.set_timeout(delay = 1000,
                  on_triggered = [] { std::cout << "1.0 second Timeout!" << std::endl; });
    c.set_timeout(delay = 2500,
                  on_triggered = [] { std::cout << "2.5 second Timeout!" << std::endl; });
    c.set_timeout(delay = 5000,
                  on_triggered = [] { std::cout << "5.0 second Timeout!" << std::endl; });
    c.set_timeout(delay = 6000,
                  on_triggered = [&] { 
                      std::cout << "Stopping component" << std::endl; 
                      c.stop();
                    });
    c.start();
}