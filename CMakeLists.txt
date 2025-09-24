cmake_minimum_required(VERSION 3.12)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->)
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(RayTracing)
set(CMAKE_CXX_STANDARD 20)


set(IMGUI 
imgui
imgui/backends
)
set(GLSHADER GLShader)
set(RENDERPASS RenderPass)
set(UI UI)

file(GLOB_RECURSE IMGUI_SRC 
    imgui/*.cpp
)

file(GLOB_RECURSE SHADER_SRC
    GLShader/*.cpp
)

file(GLOB_RECURSE PASS_SRC
    RenderPass/*.cpp
)


include_directories(${IMGUI})
include_directories(${GLSHADER})
include_directories(${RENDERPASS})
include_directories(${UI})

add_executable(${PROJECT_NAME} "main.cpp" "InputHandler.cpp" "DebugObjectRenderer.cpp" ${IMGUI_SRC} ${SHADER_SRC} ${PASS_SRC})

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

find_package(OpenGL REQUIRED) 
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)