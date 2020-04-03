cmake_minimum_required(VERSION 3.10)

project(Playground VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)

# Temporary files (like object files) created while compiling projects.
set(tmp_dir ${CMAKE_CURRENT_BINARY_DIR}/obj)

# Output executables in `bin`, and libraries (both static and module) in `obj`.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${tmp_dir})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${tmp_dir})

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}) 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}) 

# SDL
message( ${CMAKE_SYSTEM_NAME} )
message( ${PROJECT_SOURCE_DIR}/external/${CMAKE_SYSTEM_NAME}/libs/sdl2/include)
set(SDL2_ROOT ${PROJECT_SOURCE_DIR}/external/${CMAKE_SYSTEM_NAME}/libs/sdl2)
set(SDL2_INCLUDE_DIRS "${SDL2_ROOT}/include")
set(SDL2_LIBS "${SDL2_ROOT}/lib/x64/SDL2.lib")
set(SDL2_LIBS_Main "${SDL2_ROOT}/lib/x64/SDL2main.lib")
set(SDL2_DLLS "${SDL2_ROOT}/lib/x64/SDL2.dll")

include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}  ${SDL2_LIBS})

# Copy DLLs to output folder on Windows
if(WIN32)
    foreach(DLL ${SDL2_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach()
endif()