include_directories(${PROJECT_SOURCE_DIR}/server)
#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/server/net)

set(APP_SRC 
    eventloop.cpp
    channel.cpp
    poller.cpp
    eventloopthread.cpp
    inetaddress.cpp
    acceptor.cpp
    Socket.cpp
    tcpserver.cpp
    tcpconnection.cpp
    common.h
    timer.cpp
    timerqueue.cpp
    buffer.cpp
    eventloopthreadpool.cpp
    epoller.cpp
    connector.cpp
    tcpclient.cpp
    )

add_library(net ${APP_SRC})
#target_include_directories(net PUBLIC server/net)
#target_include_directories(net PUBLIC server/base)
target_link_libraries(net base)



#install(TARGETS net DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/server/net)

add_subdirectory(http)
add_subdirectory(test)
