cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
set(NAME VulkanEngine)

project(${NAME})

include_directories(external)
include_directories(external/glm)
include_directories(external/gli)
include_directories(external/imgui)
include_directories(external/tinygltf)
include_directories(external/ktx/include)
include_directories(external/ktx/other_include)
include_directories(src/base)

message(STATUS "Using module to find Vulkan:")
find_package(Vulkan)
IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

# -DNOMINMAX:禁用 Windows 系统头文件中定义的 min 和 max 宏，避免与 C++ 标准库中的 std::min/std::max 产生冲突
# -D_USE_MATH_DEFINES:启用数学常量定义（如 M_PI 等）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//bin//")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(SHADERS_OUTPUT "${CMAKE_BINARY_DIR}/bin/shaders" CACHE PATH "shader编译输出目录")

add_subdirectory(src/base)
add_subdirectory(src/engine)
add_subdirectory(shaders)
