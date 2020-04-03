<p align="center">
  <img height="150" src="img/logo.png"/>  
</p>

`iris` is a `C++17` header-only library that provides a [component model](https://en.wikipedia.org/wiki/Component-based_software_engineering) and messaging framework based on [ZeroMQ](https://zeromq.org/). 

Here's a simple publish-subscribe example:

```cpp
// publisher.cpp
#include <iostream>
#include <iris/iris.hpp>

int main() {
  iris::Component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});

  unsigned i{0};
  sender.set_interval(period = 500, 
                      on_expiry = [&] { 
                          const auto msg = "Hello World " + std::to_string(i);
                          p.send(msg);
                          std::cout << "Published " << msg << "\n";
                          ++i;
                      });
  sender.start();
}
```


```cpp
// subscriber.cpp
#include <iostream>
#include <iris/iris.hpp>

int main() {
  iris::Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
                             on_receive = [&](iris::Message msg) {
                                 std::cout << "Received "
                                           << msg.deserialize<std::string>()
                                           << "\n";
                             });
  receiver.start();
}
```

## Quick Start

Simply include `#include <iris/iris.hpp>` and you're good to go. Get started by creating an `iris::Component`. 

```cpp
iris::Component my_component;
```

Each `iris::Component` can have one or more of the following:

* Periodic and sporadic timers
* Publishers, subscribers, facets (clients), and receptacles (servers)
* State variables

### Periodic Timers

`iris` components can be triggered periodically by timers. To create a timer, call `component.set_interval`. The following component is triggered every 500ms. Timers are an excellent way to kickstart a communication pattern, e.g., publish messages periodically to multiple sinks.

```cpp
my_component.set_interval(period = 500, 
                          on_expiry = [] { std::cout << "Timer fired!\n"; });
```

