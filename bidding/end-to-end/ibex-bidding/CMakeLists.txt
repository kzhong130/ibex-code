CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

find_package(SEAL 4.0.0 REQUIRED)

INCLUDE_DIRECTORIES(../../libs)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(bidder bidder.cpp ../../libs/SealPIR/src/pir.hpp ../../libs/SealPIR/src/pir.cpp ../../libs/SealPIR/src/pir_client.hpp ../../libs/SealPIR/src/pir_client.cpp ../../libs/SealPIR/src/pir_server.hpp ../../libs/SealPIR/src/pir_server.cpp)
target_link_libraries(bidder SEAL::seal ssl crypto)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(client client.cpp ../../libs/SealPIR/src/pir.hpp ../../libs/SealPIR/src/pir.cpp ../../libs/SealPIR/src/pir_client.hpp ../../libs/SealPIR/src/pir_client.cpp ../../libs/SealPIR/src/pir_server.hpp ../../libs/SealPIR/src/pir_server.cpp)
target_link_libraries(client SEAL::seal ssl crypto pthread gmp)