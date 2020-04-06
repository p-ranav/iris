add_executable(music_tag_client client.cpp)
target_include_directories(music_tag_client PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(music_tag_client PRIVATE iris::iris ${ZeroMQ_LIBRARY})

add_executable(music_tag_server server.cpp)
target_include_directories(music_tag_server PRIVATE ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(music_tag_server PRIVATE iris::iris ${ZeroMQ_LIBRARY})