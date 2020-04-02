# iris

`iris` is a lightweight `C++17` header-only library that provides a component model and messaging framework based on [ZeroMQ](https://zeromq.org/). Here's a simple publish-subscribe example:

```cpp
// sender.cpp
#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>
#include <iris/timer.hpp>

int main() {
  iris::component sender;
  auto p = sender.create_publisher({"tcp://*:5555"});
  sender.set_interval(period = 50, 
                      on_expiry = [&p] { p.send("Hello, World!"); });
  sender.start();
}
```


```cpp
// receiver.cpp
#include <iostream>
#include <iris/component.hpp>
#include <iris/subscriber.hpp>

int main() {
  iris::component receiver;
  receiver.create_subscriber(
    {"tcp://localhost:5555"}, 
    on_receive = [](auto msg) { std::cout << "Received " << msg << "\n";}
  );
  receiver.start();
}
```