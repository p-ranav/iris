# iris

`iris` is a `C++17` header-only library that provides a [component model](https://en.wikipedia.org/wiki/Component-based_software_engineering) and messaging framework based on [ZeroMQ](https://zeromq.org/). `iris` supports time-triggered operations, pub-sub messaging, client-server interactions, serialization/deserialization and a multi-threaded task system with task stealing.

Here's a simple publish-subscribe example:

```cpp
// sender.cpp
#include <iostream>
#include <iris/iris.hpp>

int main() {
  iris::Component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});
  sender.set_interval(period = 50,
                      on_expiry = [&p] { p.send("Hello, World!"); });
  sender.start();
}
```


```cpp
// receiver.cpp
#include <iostream>
#include <iris/iris.hpp>

int main() {
  iris::Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
                             on_receive = [](auto msg) { std::cout << "Received " << msg << "\n"; });
  receiver.start();
}
```
