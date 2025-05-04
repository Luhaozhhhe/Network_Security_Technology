cmake_minimum_required(VERSION 3.21)
project(RSA_chat)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(RSA_chat main.cpp
        src/DES_Operation.cpp
        src/RSA_Operation.cpp
        src/chat.cpp)

target_include_directories(RSA_chat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(RSA_chat ws2_32)
endif()