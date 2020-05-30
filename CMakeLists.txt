cmake_minimum_required(VERSION 3.0)

set(SRC_LIST
    Epoller.cpp
    Channel.cpp 
    Epoller.cpp 
    EventLoop.cpp 
    NetServer.cpp
    TimerQueue.cpp
    main.cpp
)
add_definitions(-std=c++11 -g -Wall)

add_executable(epoll ${SRC_LIST})

# set(BIND_LIST
#     bind
# )
# add_executable(bind ${BIND_LIST})
#target_include_directories(echoClient 
##    PUBLIC
#    ${PROJECT_SOURCE_DIR}/echoClient
#)
link_directories(/usr/lib/muduo)

target_link_libraries(epoll 
    muduo_net
    muduo_base
    pthread)

# target_link_libraries(bind
#     muduo_base
#     pthread)