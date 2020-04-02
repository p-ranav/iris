# iris

`iris` is a lightweight `C++17` header-only library that provides a [component model](https://en.wikipedia.org/wiki/Component-based_software_engineering) and messaging framework based on [ZeroMQ](https://zeromq.org/). `iris` supports time-triggered operations, pub-sub messaging, client-server interactions, serialization/deserialization and a multi-threaded task system with task stealing.

Here's a simple publish-subscribe example:

```cpp
// sender.cpp
#include <iostream>
#include <iris/component.hpp>
#include <iris/publisher.hpp>
#include <iris/timer.hpp>

int main() {
  iris::component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});
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
  iris::component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
                             on_receive = [](auto msg) { std::cout << "Received " << msg << "\n"; });
  receiver.start();
}
```
