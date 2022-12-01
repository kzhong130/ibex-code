cmake_minimum_required (VERSION 2.8.11)
project (emp-sh2pc)
set(NAME "emp-sh2pc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})

ENABLE_TESTING()

#Testing macro
macro (add_test_executable_with_lib _name libs)
	add_executable(${_name} "${_name}.cpp")
	target_link_libraries(${_name} ${EMP-OT_LIBRARIES})
endmacro()

# Test cases
add_test_executable_with_lib(auction "")

INCLUDE_DIRECTORIES(../../libs)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(bidding_share bidding_share.cpp)
TARGET_LINK_LIBRARIES(bidding_share gmp)