file(GLOB BASE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/base/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/base/*.h"
)

file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.h"
)

add_executable(VulkanEngine
    ${ENGINE_SOURCES}
    ${BASE_SRC}
)
source_group("Base" FILES ${BASE_SRC})
source_group("Engine" FILES ${ENGINE_SOURCES})

target_link_libraries(VulkanEngine
    PRIVATE
    Vulkan::Vulkan
    ${WINLIBS}
    imgui
    ktx
)

set_target_properties(VulkanEngine
    PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    WIN32_EXECUTABLE ON
)

target_compile_definitions(VulkanEngine
    PRIVATE
    ENGINE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    SHADERS_SPV_DIR="${SHADERS_OUTPUT}"
)

add_dependencies(VulkanEngine Shaders)