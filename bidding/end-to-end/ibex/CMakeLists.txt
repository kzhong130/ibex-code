cmake_minimum_required (VERSION 2.8.11)
project (emp-sh2pc)
set(NAME "emp-sh2pc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})

find_package(SEAL 4.0.0 REQUIRED)

INCLUDE_DIRECTORIES(../../libs)

add_executable(auctioneer auctioneer.cpp)
target_link_libraries(auctioneer ${EMP-OT_LIBRARIES} gmp ssl crypto pthread)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(bidder bidder.cpp ../../libs/SealPIR/src/pir.hpp ../../libs/SealPIR/src/pir.cpp ../../libs/SealPIR/src/pir_client.hpp ../../libs/SealPIR/src/pir_client.cpp ../../libs/SealPIR/src/pir_server.hpp ../../libs/SealPIR/src/pir_server.cpp)
target_link_libraries(bidder SEAL::seal ssl crypto)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(client client.cpp ../../libs/SealPIR/src/pir.hpp ../../libs/SealPIR/src/pir.cpp ../../libs/SealPIR/src/pir_client.hpp ../../libs/SealPIR/src/pir_client.cpp ../../libs/SealPIR/src/pir_server.hpp ../../libs/SealPIR/src/pir_server.cpp)
target_link_libraries(client SEAL::seal ssl crypto pthread gmp)