cmake_minimum_required(VERSION 3.10)

INCLUDE(FindProtobuf)
FIND_PACKAGE(Protobuf REQUIRED)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/proto/message.proto)
add_library(LIBTCP ${PROTO_HEADER} ${PROTO_SRC})

target_sources(LIBTCP
                PRIVATE
                    # does not need CMAKE_CURRENT_SOURCE_DIR prefix as this cpp file will only be used internally to this library
                    src/protocol.cpp
                    src/tcp-connection.cpp
                    src/tcp-server.cpp
                    src/tcp-client.cpp
                PUBLIC
                    # this is the path that other projects will use to find this file, so it needs to have CMAKE_CURRENT_SOURCE_DIR
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/protocol.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/tcp-connection.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/tcp-server.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/tcp-client.h
                )
# where can this libraries files be found
target_include_directories(LIBTCP 
                            PUBLIC
                                # generated proto files will be found here for now, cant put them in target_sources as the files do not exist yet
                                ${CMAKE_BINARY_DIR}/lib
                                # when building this library this is the location it will be found
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lib>
                                $<INSTALL_INTERFACE:include/lib>)  # when installing the library this is the lcoation <prefix>/include/mylib
target_compile_definitions(LIBTCP PRIVATE _WIN32_WINNT=0x0A00)      # fixes the "Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately." warning
install(TARGETS LIBTCP DESTINATION ${PROJECT_BINARY_DIR}/TEMP/lib) # put the library in this location