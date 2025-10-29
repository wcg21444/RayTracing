cmake_minimum_required(VERSION 3.12)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->)
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(RayTracing)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY = ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")


set(IMGUI 
imgui
imgui/backends
)

file(GLOB_RECURSE IMGUI_SRC 
    imgui/*.cpp
)

file(GLOB_RECURSE SHADER_SRC
    GLShader/*.cpp
)

file(GLOB_RECURSE PASS_SRC
    RenderPass/*.cpp
)
file(GLOB_RECURSE RENDERER_SRC
    Renderer/*.cpp
)
file(GLOB_RECURSE PROGRAM_SRC
    Program/*.cpp
)

file(GLOB_RECURSE CPURAYTRACE_SRC
    CPURayTrace/*.cpp
)


include_directories(${IMGUI})
include_directories("GLShader")
include_directories("RenderPass")
include_directories("Renderer")
include_directories("Program")
include_directories("CPURayTrace")
include_directories("Include")



add_executable(${PROJECT_NAME}
"main.cpp"
${IMGUI_SRC}
${SHADER_SRC}
${PASS_SRC}
${RENDERER_SRC}
${PROGRAM_SRC}
${CPURAYTRACE_SRC}
       )

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})


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

# find_package(Doxygen)
#     if (DOXYGEN_FOUND)
#         set(DOXYGEN_INPUT_SOURCES 
#             ${SHADER_SRC} 
#             ${PASS_SRC} 
#             ${RENDERER_SRC}
#             ${PROGRAM_SRC}
#             ${CPURAYTRACE_SRC}
#         )
#         set(DOXYGEN_INPUT_DIRS 
#         "${CMAKE_CURRENT_SOURCE_DIR}/GLShader"
#         "${CMAKE_CURRENT_SOURCE_DIR}/RenderPass"
#         "${CMAKE_CURRENT_SOURCE_DIR}/Renderer"
#         "${CMAKE_CURRENT_SOURCE_DIR}/Program"
#         "${CMAKE_CURRENT_SOURCE_DIR}/CPURayTrace"
#         "${CMAKE_CURRENT_SOURCE_DIR}/Include" # 假设这是放置通用头文件的目录
#     )
# list(JOIN DOXYGEN_INPUT_SOURCES " " DOXYGEN_INPUT_SOURCES_STRING)

# # ----------------------------------------------------
# # 2. 对目录列表进行 JOIN
# #    DOXYGEN_INPUT_DIRS 必须在此之前被定义和填充
# # ----------------------------------------------------
# list(JOIN DOXYGEN_INPUT_DIRS " " DOXYGEN_INPUT_DIRS_STRING)

# # ----------------------------------------------------
# # 3. 合并为最终的 INPUT 变量
# #    确保使用双引号括起来以正确处理空格
# # ----------------------------------------------------
# set(DOXYGEN_INPUT 
#     "${DOXYGEN_INPUT_SOURCES_STRING} ${DOXYGEN_INPUT_DIRS_STRING}"
# )
#     # 定义 Doxyfile 的路径，最好放在构建目录中，以便管理
#     set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
#     set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)  

#     # ----------------------------------------------------
#     # 1. 配置 Doxyfile 模板
#     # ----------------------------------------------------
#     # Doxyfile.in 是一个模板文件，其中包含 Doxygen 的配置。
#     # 我们使用 CMake 变量来替换模板中的一些路径。
#     configure_file(
#         ${DOXYFILE_IN}
#         ${DOXYFILE_OUT}
#         @ONLY
#     )

#     # ----------------------------------------------------
#     # 2. 定义 Doxygen 构建目标
#     # ----------------------------------------------------
#     # 创建一个名为 'doc' 的目标，用于运行 Doxygen
#     add_custom_target(doc ALL
#         COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         COMMENT "Generating documentation with Doxygen..."
#         USES_TERMINAL
#     )

#     # 可选：将文档目标设置为构建所有目标时的依赖
#     # set_target_properties(doc PROPERTIES EXCLUDE_FROM_ALL TRUE) # 默认不构建，需要手动 make doc

#     message(STATUS "Doxygen 'doc' target enabled. Run 'cmake --build . --target doc' to generate docs.")

# else()
#     message(WARNING "Doxygen not found. Documentation will not be generated.")
# endif()