add_executable(periodic_timers periodic_timers.cpp)
target_include_directories(periodic_timers PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(periodic_timers PRIVATE iris::iris ${ZeroMQ_LIBRARY})
