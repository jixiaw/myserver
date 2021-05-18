include_directories(${PROJECT_SOURCE_DIR}/netjxw)
add_executable(util_test test_util.cpp)
target_link_libraries(util_test base)

add_executable(timestamp_test timestamp_test.cpp)
target_link_libraries(timestamp_test base)


