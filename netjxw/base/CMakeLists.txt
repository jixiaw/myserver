include_directories(${PROJECT_SOURCE_DIR}/netjxw)
set(LIB_UTIL
    timestamp.cpp
    string_util.cpp
    time_util.cpp
    logging.cpp
    logstream.h
    threadpool.cpp
    fileutil.cpp
    )

add_library(base ${LIB_UTIL})

add_subdirectory(test)

