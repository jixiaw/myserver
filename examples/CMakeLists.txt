include_directories(${PROJECT_SOURCE_DIR}/server)
add_executable(echo_server echo_server.cpp)
target_link_libraries(echo_server net)
target_link_libraries(echo_server ${CMAKE_THREAD_LIBS_INIT})


add_executable(file_httpdownload file_httpdownload.cpp)
target_link_libraries(file_httpdownload net)
target_link_libraries(file_httpdownload http)
target_link_libraries(file_httpdownload ${CMAKE_THREAD_LIBS_INIT})

add_executable(http_server http_server.cpp)
target_link_libraries(http_server net)
target_link_libraries(http_server http)
target_link_libraries(http_server ${CMAKE_THREAD_LIBS_INIT})