cmake_minimum_required(VERSION 3.10)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

find_package(SEAL 4.0.0 REQUIRED)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(..)
ADD_EXECUTABLE(client client.cpp ../SealPIR/src/pir.hpp ../SealPIR/src/pir.cpp ../SealPIR/src/pir_client.hpp ../SealPIR/src/pir_client.cpp ../SealPIR/src/pir_server.hpp ../SealPIR/src/pir_server.cpp)
target_link_libraries(client SEAL::seal ssl crypto pthread gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(..)
ADD_EXECUTABLE(client_multi client_multi.cpp ../SealPIR/src/pir.hpp ../SealPIR/src/pir.cpp ../SealPIR/src/pir_client.hpp ../SealPIR/src/pir_client.cpp ../SealPIR/src/pir_server.hpp ../SealPIR/src/pir_server.cpp)
target_link_libraries(client_multi SEAL::seal ssl crypto pthread gmp)
