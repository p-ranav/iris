add_executable(oneshot_timers oneshot_timers.cpp)
target_include_directories(oneshot_timers PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(oneshot_timers PRIVATE iris::iris ${ZeroMQ_LIBRARY})
