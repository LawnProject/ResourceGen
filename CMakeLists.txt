# CMakeLists.txt
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(GardenersDecomp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_subdirectory(thirdparty)

add_subdirectory(src)