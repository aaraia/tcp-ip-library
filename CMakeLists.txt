cmake_minimum_required(VERSION 3.10)
project(TCP-LIB VERSION 1.0)
set(CMAKE_CXX_STANDARD 11)
set(Boost_USE_STATIC_LIBS ON)   # AA: NOTE: if Boost_USE_STATIC_LIBS is set to OFF, find_package will not be able to find libraries
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS filesystem) # this actually finds boost!!
find_package(Boost REQUIRED COMPONENTS date_time)
find_package(Boost REQUIRED COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS}) 
add_subdirectory(lib)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(tests)