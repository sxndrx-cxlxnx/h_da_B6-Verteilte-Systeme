cmake_minimum_required(VERSION 3.5)

project(Wettanbieter)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H")

set(THRIFT_DIR "/usr/local/include/thrift")

include_directories(${THRIFT_DIR}} ${CMAKE_SOURCE_DIR})
link_directories(/usr/local/lib)

set(BASE_SOURCE_FILES Wettanbieter.cpp Wettanbieter_types.h)
set(CLIENT_FILES CppClient.cpp)

add_executable(CppClient ${CLIENT_FILES} ${BASE_SOURCE_FILES})

target_link_libraries(CppClient thrift)