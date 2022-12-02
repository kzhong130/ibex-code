cmake_minimum_required (VERSION 2.8.11)
project (emp-sh2pc)
set(NAME "emp-sh2pc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(../../../libs)

add_executable(auctioneer auctioneer.cpp)
target_link_libraries(auctioneer ${EMP-OT_LIBRARIES} gmp ssl crypto pthread)