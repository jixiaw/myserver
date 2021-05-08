include_directories(${PROJECT_SOURCE_DIR}/netjxw)
#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/netjxw/net)

set(APP_SRC 
    eventloop.cpp
    channel.cpp
    poller.cpp
    eventloopthread.cpp
    inetaddress.cpp
    acceptor.cpp
    Socket.cpp
    )

add_library(net ${APP_SRC})
#target_include_directories(net PUBLIC netjxw/net)
#target_include_directories(net PUBLIC netjxw/base)
target_link_libraries(net base)



#install(TARGETS net DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/netjxw/net)

add_subdirectory(test)
