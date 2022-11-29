CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(gm_test gm_test.cpp)
TARGET_LINK_LIBRARIES(gm_test gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(auction_enc_test auction_enc_test.cpp)
TARGET_LINK_LIBRARIES(auction_enc_test gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(key_gen key_gen.cpp)
TARGET_LINK_LIBRARIES(key_gen gmp)