cmake_minimum_required(VERSION 3.10)

project(HeiTwin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(
  ./src
)

# add_compile_definitions(ENABLE_LOG)

add_library(utils src/utils.cc)
add_library(graph src/data_structures/graph.cc)

add_executable(HeiTwin src/HeiTwin.cc)

target_link_libraries(HeiTwin PUBLIC utils)
target_link_libraries(HeiTwin PUBLIC graph)
