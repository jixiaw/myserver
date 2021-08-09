include_directories(${PROJECT_SOURCE_DIR}/server)
add_executable(util_test test_util.cpp)
target_link_libraries(util_test base)

add_executable(timestamp_test timestamp_test.cpp)
target_link_libraries(timestamp_test base)

add_executable(logging_test logging_test.cpp)
target_link_libraries(logging_test base)
target_link_libraries(logging_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(threadpool_test threadpool_test.cpp)
target_link_libraries(threadpool_test base)
target_link_libraries(threadpool_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(fileutil_test fileutil_test.cpp)
target_link_libraries(fileutil_test base)

add_executable(codecutil_test codecutil_test.cpp)
target_link_libraries(codecutil_test base)