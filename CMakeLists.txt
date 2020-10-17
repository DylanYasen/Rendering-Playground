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

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)

set(SOURCES
   ${PROJECT_SOURCE_DIR}/FileUtils.h
   ${PROJECT_SOURCE_DIR}/FileUtils.cpp
   ${PROJECT_SOURCE_DIR}/gl.cpp
   ${PROJECT_SOURCE_DIR}/gl.h
   ${PROJECT_SOURCE_DIR}/Math.h
   ${PROJECT_SOURCE_DIR}/IndexBuffer.h
   ${PROJECT_SOURCE_DIR}/IndexBuffer.cpp
   ${PROJECT_SOURCE_DIR}/Shader.h
   ${PROJECT_SOURCE_DIR}/Shader.cpp
   ${PROJECT_SOURCE_DIR}/Texture.h
   ${PROJECT_SOURCE_DIR}/Texture.cpp
   ${PROJECT_SOURCE_DIR}/VertexBufferLayout.h
   ${PROJECT_SOURCE_DIR}/VertexArray.h
   ${PROJECT_SOURCE_DIR}/VertexArray.cpp
   ${PROJECT_SOURCE_DIR}/VertexBuffer.h
   ${PROJECT_SOURCE_DIR}/VertexBuffer.cpp
   ${PROJECT_SOURCE_DIR}/Renderer.h
   ${PROJECT_SOURCE_DIR}/Renderer.cpp

   ${PROJECT_SOURCE_DIR}/Camera.h
   ${PROJECT_SOURCE_DIR}/Camera.cpp

   ${PROJECT_SOURCE_DIR}/Transform.h
   ${PROJECT_SOURCE_DIR}/Transform.cpp

   ${PROJECT_SOURCE_DIR}/Asset.h
   ${PROJECT_SOURCE_DIR}/Asset.cpp
   ${PROJECT_SOURCE_DIR}/Mesh.h
   ${PROJECT_SOURCE_DIR}/Mesh.cpp
   ${PROJECT_SOURCE_DIR}/Renderable.h
   ${PROJECT_SOURCE_DIR}/Renderable.cpp

   ${PROJECT_SOURCE_DIR}/Scene.h
   ${PROJECT_SOURCE_DIR}/Scene.cpp

   ${PROJECT_SOURCE_DIR}/Light.h
   ${PROJECT_SOURCE_DIR}/Light.cpp

   ${PROJECT_SOURCE_DIR}/FrameData.h
   ${PROJECT_SOURCE_DIR}/FrameData.cpp

   ${PROJECT_SOURCE_DIR}/Allocator.h
   ${PROJECT_SOURCE_DIR}/Allocator.cpp
   
   ${PROJECT_SOURCE_DIR}/external/stb_image.cpp
   ${PROJECT_SOURCE_DIR}/external/handmade_math.cpp

   external/src/imgui/imgui.cpp
   external/src/imgui/imgui_draw.cpp
   external/src/imgui/imgui_widgets.cpp
   external/src/imgui/examples/imgui_impl_opengl3.cpp
   external/src/imgui/examples/imgui_impl_sdl.cpp
   
   ${PROJECT_SOURCE_DIR}/main.cpp   
)

add_executable(${PROJECT_NAME} ${SOURCES})

# SDL
add_subdirectory(external/src/sdl)
set(SDL_STATIC OFF)
include_directories(external/src/sdl/include)
target_link_libraries(${PROJECT_NAME} SDL2)

# GLAD
add_subdirectory(external/src/glad)
set(GLAD_ALL_EXTENSIONS ON)
include_directories(external/src/glad/include/glad)
target_link_libraries(${PROJECT_NAME} glad)

# D3D11
if(WIN32)
    target_sources(${PROJECT_NAME} 
        PRIVATE
            src/pixel.hlsl
            src/vertex.hlsl
        )
   
    set(ShaderModel 5.0)
    set_source_files_properties(src/vertex.hlsl PROPERTIES VS_SHADER_FLAGS "/Fh \"$(OutDir)%(Filename).h\"" VS_SHADER_TYPE Vertex VS_SHADER_MODEL ${ShaderModel} VS_SHADER_ENTRYPOINT vert)
	set_source_files_properties(src/pixel.hlsl PROPERTIES VS_SHADER_FLAGS "/Fh \"$(OutDir)%(Filename).h\"" VS_SHADER_TYPE Pixel  VS_SHADER_MODEL ${ShaderModel} VS_SHADER_ENTRYPOINT frag)
    target_link_libraries(${PROJECT_NAME} d3d11 D3DCompiler)

      # work dir for visual studio
    #   set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
endif()

# Vulkan
find_package(Vulkan)
if(VULKAN_FOUND)
    # add_definitions(VULKAN)
    include_directories(${VULKAN_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${VULKAN_LIBRARY})
    if(WIN32)
    # post build hook for transpiling glsl to spv
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND call "$(SolutionDir)scripts/transpileGlsl.bat" "$(OutDir)shaders")
    endif()
endif(VULKAN_FOUND)

# std headers
include_directories(external/src/stb)

# handmade math headers
include_directories(external/src/math)

# assimp
# todo: figure out a way to override the destination for this shit.  ASSIMP_BIN_INSTALL_DIR is a cached var
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
# set(ASSIMP_BIN_INSTALL_DIR  ${EXECUTABLE_OUTPUT_PATH})
add_subdirectory(external/src/assimp)
target_link_libraries(${PROJECT_NAME} assimp)


# imgui
include_directories(external/src/imgui)
include_directories(external/src/imgui/examples)