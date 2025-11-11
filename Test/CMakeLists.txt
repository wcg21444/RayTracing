cmake_minimum_required(VERSION 3.15)


add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->)
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)


set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(TestMainProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." CACHE PATH "Project root path")

file(GLOB_RECURSE IMGUI_SRC 
    ../imgui/*.cpp
)

file(GLOB_RECURSE SHADER_SRC
    ../GLShader/*.cpp
)

file(GLOB_RECURSE PASS_SRC
    ../RenderPass/*.cpp
)
file(GLOB_RECURSE RENDERER_SRC
    ../Renderer/*.cpp
)
file(GLOB_RECURSE PROGRAM_SRC
    ../Program/*.cpp
)

file(GLOB_RECURSE CPURAYTRACE_SRC
    ../CPURayTrace/*.cpp
)


include_directories("../GLShader")
include_directories("../RenderPass")
include_directories("../Renderer")
include_directories("../Program")
include_directories("../CPURayTrace")
include_directories("../Include")

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
add_executable(${PROJECT_NAME} 
${TEST_SOURCES}
${IMGUI_SRC}
${SHADER_SRC}
${PASS_SRC}
${RENDERER_SRC}
${PROGRAM_SRC}
${CPURAYTRACE_SRC}
 )

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad::glad
    glm::glm
    OpenGL::GL
    GLEW::GLEW
    assimp::assimp
)