add_executable(publisher publisher.cpp)
target_include_directories(publisher PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(publisher PRIVATE iris::iris ${ZeroMQ_LIBRARY})

add_executable(subscriber subscriber.cpp)
target_include_directories(subscriber PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(subscriber PRIVATE iris::iris ${ZeroMQ_LIBRARY})