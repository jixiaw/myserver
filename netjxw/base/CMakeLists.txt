include_directories(${PROJECT_SOURCE_DIR}/netjxw)
set(LIB_UTIL
    timestamp.cpp
    string_util.cpp
    time_util.cpp
    )

add_library(base ${LIB_UTIL})

add_subdirectory(test)

#target_include_directories(base PUBLIC netjxw/base)

#target_link_libraries(base pthread rt)

#install(TARGETS base DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/netjxw/base)

