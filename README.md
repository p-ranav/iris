<p align="center">
  <img height="150" src="img/logo.png"/>  
</p>

`iris` is a `C++17` header-only library that provides a [component model](https://en.wikipedia.org/wiki/Component-based_software_engineering) and messaging framework based on [ZeroMQ](https://zeromq.org/). 

## Component Model

Here's the anatomy of an `iris::Component`. `iris` components can have a variety of ports and timers. There are 4 basic types of ports: ***publisher***, ***subscriber***, ***client***, and ***server*** ports. Publisher ports publish messages, without blocking, on specific endpoints. Subscriber ports subscribe to such topics (on specific endpoints) and receive messages published by one or more publishers. Server ports provide an interface to a component service. Client ports can use this interface to request such services. Component timers can be periodic or sporadic and allow components to trigger themselves with the specified timing characteristics.

An _operation_ is an abstraction for the different tasks undertaken by a component.  These tasks are implemented by the component’s source code written by the developer. Application developers provide the functional, business-logic code that implements operations on local state variables and inputs received on component ports. 

Operation requests (e.g., timer expired so please call my callback) are serviced by one or more executor threads that make up the component's task system. 

<p align="center">
  <img height="600" src="img/iriscom.png"/>  
</p>

## Getting Started

Simply include `#include <iris/iris.hpp>` and you're good to go. Start by creating an `iris::Component`:

```cpp
iris::Component my_component;
```

You can optionally specify the number of threads the component can use in its task system, e.g., this component will spawn 2 executor threads that process records in its message queues. 

```cpp
iris::Component my_component(iris::threads = 2);
```

**NOTE:** Here `iris::threads` is a [NamedType](https://github.com/joboccara/NamedType) parameter. It is not necessary to use named parameters but it certain cases, they improve code readability, e.g.,:

```cpp
using namespace iris;
// Works but not very readable:
// component.create_subscriber("tcp://localhost:5555", "Foo", 500, [] (Message msg) {});

// More readable:
component.create_subscriber(endpoints = {"tcp://localhost:5555"},
                            filter = "Foo",
                            timeout = 500,
                            on_receive = [] (Message msg) {}
                            );

```

## Time-Triggered Operations

`iris` components can be triggered periodically by timers. To create a timer, call `component.set_interval`. The following component is triggered every 500ms. Timers are an excellent way to kickstart a communication pattern, e.g., publish messages periodically to multiple sinks.

```cpp
#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component my_component;
  my_component.set_interval(period = 500,
                            on_expiry = [] { std::cout << "Timer fired!\n"; });
  my_component.start();
}
```

## Publish-Subscribe Interactions

Here's a simple publish-subscribe example. Let's start with the publisher - This is a time-triggered `sender` that publishes messages every 500ms. 

```cpp
// publisher.cpp
#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component sender;
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

The `receiver` component is subscribes to messages on the endpoint `tcp://localhost:5555`. Subscriber callbacks have the signature `std::function<void(iris::Message)>`. `iris` uses [Cereal](https://uscilab.github.io/cereal/) to serialize messages. Use `Message.get<T>` to get received messages. 

```cpp
// subscriber.cpp
#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

int main() {
  Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
                             on_receive = [](Message msg) {
                                 std::cout << "Received "
                                           << msg.get<std::string>()
                                           << "\n";
                             });
  receiver.start();
}
```
