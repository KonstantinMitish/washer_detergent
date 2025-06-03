cmake_minimum_required(VERSION 3.25)
project(washer_detergent_client)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

add_executable(washer_detergent client.cpp)

target_include_directories(washer_detergent PRIVATE ${OpenSSL_INCLUDE_DIR})
target_link_libraries(washer_detergent PRIVATE 
        OpenSSL::Crypto
        OpenSSL::SSL)