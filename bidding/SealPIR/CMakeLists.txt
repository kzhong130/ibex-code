cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")
project(SealPIR VERSION 2.2 LANGUAGES CXX)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#add_subdirectory(src)

#enable_testing()
#add_subdirectory(test)

find_package(SEAL 4.0.0 REQUIRED)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(net_server net_server.cpp src/pir.hpp src/pir.cpp src/pir_client.hpp src/pir_client.cpp src/pir_server.hpp src/pir_server.cpp)
target_link_libraries(net_server SEAL::seal ssl crypto)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(pir_servers pir_servers.cpp src/pir.hpp src/pir.cpp src/pir_client.hpp src/pir_client.cpp src/pir_server.hpp src/pir_server.cpp)
target_link_libraries(pir_servers SEAL::seal ssl crypto pthread)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(net_client net_client.cpp src/pir.hpp src/pir.cpp src/pir_client.hpp src/pir_client.cpp src/pir_server.hpp src/pir_server.cpp)
target_link_libraries(net_client SEAL::seal ssl crypto)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(micro_test micro_test.cpp src/pir.hpp src/pir.cpp src/pir_client.hpp src/pir_client.cpp src/pir_server.hpp src/pir_server.cpp)
target_link_libraries(micro_test SEAL::seal ssl crypto)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(micro_multi_test micro_multi_test.cpp src/pir.hpp src/pir.cpp src/pir_client.hpp src/pir_client.cpp src/pir_server.hpp src/pir_server.cpp)
target_link_libraries(micro_multi_test SEAL::seal ssl crypto pthread)