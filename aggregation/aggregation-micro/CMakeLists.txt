find_package(SEAL 4.0 REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(aggregator_micro_test aggregator_micro_test.cpp)
target_link_libraries(aggregator_micro_test SEAL::seal ssl crypto)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(he_test he_test.cpp)
target_link_libraries(he_test SEAL::seal ssl crypto)
