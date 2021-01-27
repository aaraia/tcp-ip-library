cmake_minimum_required(VERSION 3.10)
add_executable(SERVER)
target_sources(SERVER 
                PRIVATE
                    include/protocol-a.h
                    src/main.cpp
                    src/protocol-a.cpp
                )       
target_include_directories(SERVER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)                    
list(APPEND EXTRA_LIBS LIBTCP)
list(APPEND EXTRA_LIBS Boost::filesystem)
list(APPEND EXTRA_LIBS Boost::date_time)
list(APPEND EXTRA_LIBS Boost::regex)
target_link_libraries(SERVER PRIVATE ${EXTRA_LIBS})
install(TARGETS SERVER DESTINATION ${PROJECT_BINARY_DIR}/TEMP/bin) # put this library in this location