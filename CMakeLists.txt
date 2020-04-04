cmake_minimum_required(VERSION 3.10)

project(Playground VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)

set(tmp_dir ${CMAKE_CURRENT_BINARY_DIR}/obj)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${tmp_dir})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${tmp_dir})

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}) 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}) 

add_executable(${PROJECT_NAME} src/main.cpp)

# SDL
set(SDL_STATIC false)
add_subdirectory(external/src/sdl)
include_directories(external/src/sdl/include)
target_link_libraries(${PROJECT_NAME} SDL2)
