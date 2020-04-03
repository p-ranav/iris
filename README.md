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
// component.create_subscriber(std::vector<std::string>{"tcp://localhost:5555"}, 
//                             "Foo", 
//                             5000, 
//                             [] (Message msg) {});

// More readable:
component.create_subscriber(endpoints = {"tcp://localhost:5555"},
                            filter = "Foo",
                            timeout = 5000,
                            on_receive = [] (Message msg) {}
                            );

```

## Time-Triggered Operations

`iris` components can be triggered periodically by timers. To create a timer, call `component.set_interval`. The following component is triggered every 500ms. Timers are an excellent way to kickstart a communication pattern, e.g., publish messages periodically to multiple sinks.

Call `Component.start()` to start the component - This starts the component executor threads, listener threads, timers etc.

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

Here's a simple publish-subscribe example. 

Let's say we want to periodically publish `Mouse` position and our struct looks like this: 

```cpp
struct Mouse {
  int x, y;
};
```

`iris` uses [Cereal](https://github.com/USCiLab/cereal) library for serialization and deserialization of user-defined structures. Currently, `iris` uses the `cereal::PortalBinary{Input/Output}Archive` for serialization and deserialization. To enable serialization of `Mouse` objects, write a save method like below:

```cpp
struct Mouse {
  int x, y;

  // Method for serialization
  template <typename Archive>
  void save(Archive& ar) const {
    ar(x, y);
  }
};
```

Now we can create an `iris::Publisher` using `component.create_publisher` and publish periodically.

```cpp
// publisher.cpp
#include <iostream>
#include <random>
#include <iris/iris.hpp>
using namespace iris;

struct Mouse {
  int x, y;

  // Method for serialization
  template <typename Archive>
  void save(Archive& ar) const {
    ar(x, y);
  }
};

int main() {
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<> x_dist(0, 1920); // random number generator for Mouse X position
  std::uniform_int_distribution<> y_dist(0, 1080); // random number generatof for Mouse Y position

  Component sender;
  auto p = sender.create_publisher(endpoints = {"tcp://*:5555"});

  sender.set_interval(period = 250, 
    on_expiry = [&] { 
        Mouse position {.x = x_dist(eng), .y = y_dist(eng)};
        // Publish the position
        p.send(position);
        std::cout << "Published (" << position.x << ", " << position.y << ")\n";
  });
  sender.start();
}
```

Now let's write a `recevier` component. Our `receiver` component will subscribe to messages on the endpoint `tcp://localhost:5555`.

Define the `Mouse` struct on the receiver and write a `load` method to enable deserialization. 

Subscriber callbacks have the signature `std::function<void(iris::Message)>`, i.e., subscriber receives `iris::Message` objects in its callback. Simply call `messsage.get<T>` to deserialize to the type `T`. 

```cpp
// subscriber.cpp
#include <iostream>
#include <iris/iris.hpp>
using namespace iris;

struct Mouse {
  int x, y;

  // Method for deserialization
  template <typename Archive>
  void load(Archive& ar) {
    ar(x, y);
  }
};

int main() {
  Component receiver(threads = 2);
  receiver.create_subscriber(endpoints = {"tcp://localhost:5555"},
    on_receive = [](Message msg) {
      auto position = msg.get<Mouse>();
      std::cout << "Received ("
        << position.x << ", " << position.y << ")\n";
  });
  receiver.start();
}
```
