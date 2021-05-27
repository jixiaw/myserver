include_directories(${PROJECT_SOURCE_DIR}/netjxw)
add_executable(eventloop_test eventloop_test.cpp)
target_link_libraries(eventloop_test net)
target_link_libraries(eventloop_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(eventloop_test2 eventloop_test2.cpp)
target_link_libraries(eventloop_test2 net)
target_link_libraries(eventloop_test2 ${CMAKE_THREAD_LIBS_INIT})

add_executable(eventloopthread_test eventloopthread_test.cpp)
target_link_libraries(eventloopthread_test net)
target_link_libraries(eventloopthread_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(inetaddress_test inetaddress_test.cpp)
target_link_libraries(inetaddress_test net)

add_executable(acceptor_test acceptor_test.cpp)
target_link_libraries(acceptor_test net)

add_executable(tcpserver_test tcpserver_test.cpp)
target_link_libraries(tcpserver_test net)
target_link_libraries(tcpserver_test ${CMAKE_THREAD_LIBS_INIT})


add_executable(timer_test timer_test.cpp)
target_link_libraries(timer_test net)

add_executable(buffer_test buffer_test.cpp)
target_link_libraries(buffer_test net)

add_executable(echoserver_test echoserver_test.cpp)
target_link_libraries(echoserver_test net)
target_link_libraries(echoserver_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(eventloopthreadpool_test eventloopthreadpool_test.cpp)
target_link_libraries(eventloopthreadpool_test net)
target_link_libraries(eventloopthreadpool_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(tcpclient_test tcpclient_test.cpp)
target_link_libraries(tcpclient_test net)
target_link_libraries(tcpclient_test ${CMAKE_THREAD_LIBS_INIT})