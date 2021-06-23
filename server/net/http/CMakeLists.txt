include_directories(${PROJECT_SOURCE_DIR}/server)

set(APP_SRC 
    httpserver.cpp
    )

add_library(http ${APP_SRC})
target_link_libraries(http base)
target_link_libraries(http net)
