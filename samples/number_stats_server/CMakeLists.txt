add_executable(stats_client client.cpp)
target_include_directories(stats_client PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(stats_client PRIVATE iris::iris ${ZeroMQ_LIBRARY})

add_executable(stats_broker broker.cpp)
target_include_directories(stats_broker PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(stats_broker PRIVATE iris::iris ${ZeroMQ_LIBRARY})

add_executable(stats_server server.cpp)
target_include_directories(stats_server PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(stats_server PRIVATE iris::iris ${ZeroMQ_LIBRARY})