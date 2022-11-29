project(SecMail)
cmake_minimum_required(VERSION 3.4)

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} /usr/local")
include_directories(${CMAKE_SOURCE_DIR})

#set(CMAKE_C_FLAGS "-pthread -Wall -march=native -ofast -maes")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -O3 -ffast-math -pthread")

add_executable(key_generation key_generation.cpp paillier.c tools.c)
target_link_libraries(key_generation gmp)
add_executable(encryption encryption.cpp paillier.c tools.c)
target_link_libraries(encryption gmp)
add_executable(inner_product inner_product.cpp paillier.c tools.c)
target_link_libraries(inner_product gmp)
add_executable(decryption decryption.cpp paillier.c tools.c)
target_link_libraries(decryption gmp)