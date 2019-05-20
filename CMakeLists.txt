cmake_minimum_required(VERSION 3.12)

project(TP_http_server)

set(CMAKE_CXX_STANDARD 17)

include_directories(third_party/include)

file(GLOB_RECURSE source_list "./src/*.cpp" "./src/*.c" "./src/*.h")

add_executable(TP_http_server ${source_list})
target_link_libraries(TP_http_server pthread event)