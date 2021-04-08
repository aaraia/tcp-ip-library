cmake_minimum_required(VERSION 3.10)
add_executable(SERVER)
target_sources(SERVER 
                PRIVATE
                    src/main.cpp
                )       
target_include_directories(SERVER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)                    
list(APPEND EXTRA_LIBS LIBTCP)
list(APPEND EXTRA_LIBS Boost::filesystem)
list(APPEND EXTRA_LIBS Boost::date_time)
list(APPEND EXTRA_LIBS Boost::regex)
list(APPEND EXTRA_LIBS ${PROTOBUF_LIBRARY})
target_compile_definitions(SERVER PRIVATE _WIN32_WINNT=0x0A00) # fixes the "Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately." warning
target_link_libraries(SERVER PRIVATE ${EXTRA_LIBS})
install(TARGETS SERVER DESTINATION ${PROJECT_BINARY_DIR}/TEMP/bin) # put this library in this location