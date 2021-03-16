cmake_minimum_required(VERSION 3.10)
add_executable(CLIENT)
target_sources(CLIENT 
                PRIVATE
                    include/protocolA.h
                    src/main.cpp
                    src/protocolA.cpp
                )       
target_include_directories(CLIENT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)                    
list(APPEND EXTRA_LIBS LIBTCP)
list(APPEND EXTRA_LIBS Boost::filesystem)
list(APPEND EXTRA_LIBS Boost::date_time)
list(APPEND EXTRA_LIBS Boost::regex)
target_compile_definitions(CLIENT PRIVATE _WIN32_WINNT=0x0A00) # fixes the "Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately." warning
target_link_libraries(CLIENT PRIVATE ${EXTRA_LIBS})
install(TARGETS CLIENT DESTINATION ${PROJECT_BINARY_DIR}/TEMP/bin) # put this library in this location