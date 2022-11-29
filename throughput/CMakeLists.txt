CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(browse_client browse_client.cpp)
TARGET_LINK_LIBRARIES(browse_client pthread)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(browse_server browse_server.cpp)
TARGET_LINK_LIBRARIES(browse_server pthread)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(conversion_client conversion_client.cpp)
TARGET_LINK_LIBRARIES(conversion_client pthread)

add_definitions(-std=c++11)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(conversion_server conversion_server.cpp)
TARGET_LINK_LIBRARIES(conversion_server pthread)