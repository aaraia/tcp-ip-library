cmake_minimum_required(VERSION 3.10)
add_executable(CLIENT)
target_sources(CLIENT 
                PRIVATE
                    include/protocol-a.h
                    src/main.cpp
                    src/protocol-a.cpp
                )       
target_include_directories(CLIENT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)                    
list(APPEND EXTRA_LIBS LIBTCP)
list(APPEND EXTRA_LIBS Boost::filesystem)
list(APPEND EXTRA_LIBS Boost::date_time)
list(APPEND EXTRA_LIBS Boost::regex)
target_link_libraries(CLIENT PRIVATE ${EXTRA_LIBS})
install(TARGETS CLIENT DESTINATION ${PROJECT_BINARY_DIR}/TEMP/bin) # put this library in this location