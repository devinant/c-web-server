cmake_minimum_required(VERSION 2.8.4)
project(dv1457-httpd)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/webserver")

include_directories(webserver/include)
aux_source_directory(webserver/src SOURCE_FILES)
add_executable(httpd ${SOURCE_FILES})