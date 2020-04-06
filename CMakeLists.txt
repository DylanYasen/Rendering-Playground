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
set(SDL_STATIC OFF)
add_subdirectory(external/src/sdl)
include_directories(external/src/sdl/include)
target_link_libraries(${PROJECT_NAME} SDL2)

#GLAD
add_subdirectory(external/src/glad)
set(GLAD_ALL_EXTENSIONS ON)
include_directories(external/src/glad/include/glad)
target_link_libraries(${PROJECT_NAME} glad)

#D3D11
if(WIN32)
    target_sources(${PROJECT_NAME} 
        PRIVATE
            src/pixel.hlsl
            src/vertex.hlsl
        )
   
    set(ShaderModel 5.0)
    set_source_files_properties(src/vertex.hlsl PROPERTIES VS_SHADER_FLAGS "/Fh \"$(OutDir)%(Filename).h\"" VS_SHADER_TYPE Vertex VS_SHADER_MODEL ${ShaderModel} VS_SHADER_ENTRYPOINT vert)
	set_source_files_properties(src/pixel.hlsl PROPERTIES VS_SHADER_FLAGS "/Fh \"$(OutDir)%(Filename).h\"" VS_SHADER_TYPE Pixel  VS_SHADER_MODEL ${ShaderModel} VS_SHADER_ENTRYPOINT frag)

    # work dir for visual studio
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

    target_link_libraries(${PROJECT_NAME} d3d11 D3DCompiler)
endif()
