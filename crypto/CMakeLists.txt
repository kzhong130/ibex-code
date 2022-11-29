CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(ring_test ring_test.cpp)
TARGET_LINK_LIBRARIES(ring_test gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(lwe_test lwe_test.cpp)
TARGET_LINK_LIBRARIES(lwe_test gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(add_noise_test add_noise_test.cpp)
TARGET_LINK_LIBRARIES(add_noise_test gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(server_micro_test server_micro_test.cpp)
TARGET_LINK_LIBRARIES(server_micro_test m ssl crypto gmp)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(server_e2e server_e2e.cpp)
TARGET_LINK_LIBRARIES(server_e2e m ssl crypto gmp pthread)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(enc_afe_test enc_afe_test.cpp)
TARGET_LINK_LIBRARIES(enc_afe_test gmp)