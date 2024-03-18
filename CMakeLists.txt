cmake_minimum_required(VERSION 3.4)
project(chat)

set(CMAKE_CXX_STANDARD 11)

add_executable(server
    main_server.cpp
    Server.cpp
    Logger.cpp
)
target_link_libraries(server pthread)

add_executable(client
    main_client.cpp
    Client.cpp
    Logger.cpp
)
target_link_libraries(client pthread)
