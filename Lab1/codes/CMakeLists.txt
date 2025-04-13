cmake_minimum_required(VERSION 3.21)
project(DES_chat)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(DES_chat main.cpp
        src/DES_Operation.cpp
        src/chat.cpp)

target_include_directories(DES_chat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(DES_chat ws2_32)
endif()