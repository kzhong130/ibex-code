cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")
project(SealPIR VERSION 2.2 LANGUAGES CXX)

find_package(SEAL 4.0.0 REQUIRED)

INCLUDE_DIRECTORIES(../../libs)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(pir_micro pir_micro.cpp ../../libs/SealPIR/src/pir.hpp ../../libs/SealPIR/src/pir.cpp ../../libs/SealPIR/src/pir_client.hpp ../../libs/SealPIR/src/pir_client.cpp ../../libs/SealPIR/src/pir_server.hpp ../../libs/SealPIR/src/pir_server.cpp)
target_link_libraries(pir_micro SEAL::seal ssl crypto)