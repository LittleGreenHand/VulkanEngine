cmake_minimum_required (VERSION 3.10)

# 设置 slang 编译脚本路径
set(COMPILE_SHADERS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/compileshaders.py")
# 设置 slang 编译器路径
set(SLANG_COMPILER "${PROJECT_SOURCE_DIR}/external/slang/bin/slangc.exe" CACHE FILEPATH "Path to slangc compiler")

file(GLOB_RECURSE SLANG_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.slang"
)
file(GLOB_RECURSE SLANG_SOURCE_SCRIPTS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.py"
)

find_package (Python3 REQUIRED COMPONENTS Interpreter)
message(STATUS "Python 解释器路径: ${Python3_EXECUTABLE}")
add_custom_target(
    Shaders
    COMMAND ${Python3_EXECUTABLE} ${COMPILE_SHADERS_SCRIPT} --slangc "${SLANG_COMPILER}" --outputDir "${SHADERS_OUTPUT}"
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(SHADER_FILES
    ${SLANG_SOURCE_FILES}
    ${SLANG_SOURCE_SCRIPTS}
)
set_target_properties(Shaders PROPERTIES SOURCES "${SHADER_FILES}")
source_group("SLANG" FILES ${SLANG_SOURCE_FILES})
source_group("SCRIPTS" FILES ${SLANG_SOURCE_SCRIPTS})

