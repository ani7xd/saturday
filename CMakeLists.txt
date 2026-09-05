cmake_minimum_required( VERSION 4.0 )

set( CMAKE_CXX_STANDARD 26 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS ON )

project( Alice LANGUAGES CXX )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build" )

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable( alice
                main.cpp 
                ${SOURCES} )

target_include_directories( alice PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include )

target_link_libraries( alice curl simdjson mariadb yyjson b64 lexbor asound llama )