
file(GLOB_RECURSE ENGINE_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# --------------------------
# 生成可执行文件（若为库项目，可改为add_library）
add_executable(VulkanEngine
    ${ENGINE_SOURCES}
)

target_link_libraries(${PROJECT_NAME} 
    ${Vulkan_LIBRARY}
    ${WINLIBS}
    base
)

# 包含目录
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${Vulkan_INCLUDE_DIRS}
)

# 调试工作目录设置（方便运行调试）
set_target_properties(${PROJECT_NAME} 
    PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/
    WIN32_EXECUTABLE ON
)

# 将路径通过预处理器宏传递给目标
target_compile_definitions(${PROJECT_NAME}  PRIVATE
    ENGINE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"  # 传递资源目录
    SHADERS_SPV_DIR="${SHADERS_OUTPUT}"  # 传递shader输出目录
)
add_dependencies(${PROJECT_NAME} Shaders)