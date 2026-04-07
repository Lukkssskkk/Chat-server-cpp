cmake_minimum_required(VERSION 3.10)

project(chat-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_executable(chat-client-cpp src/chat_client.cpp)
add_executable(chat-server-cpp src/chat_server.cpp)

target_link_libraries(chat-client-cpp PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(chat-server-cpp PRIVATE Threads::Threads)

install(TARGETS chat-client-cpp chat-server-cpp RUNTIME DESTINATION /usr/bin)