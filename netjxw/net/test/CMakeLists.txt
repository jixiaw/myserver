include_directories(${PROJECT_SOURCE_DIR}/netjxw)
add_executable(EventLoop_test EventLoop_unittest.cpp)
target_link_libraries(EventLoop_test net)
target_link_libraries(EventLoop_test ${CMAKE_THREAD_LIBS_INIT})

