file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(${PROJECT_NAME}
    ${ALL_SOURCES}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ALL_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    ${WINLIBS}
    imgui
    ktx
    glfw
)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    WIN32_EXECUTABLE ON
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    ENGINE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    SHADERS_DIR="${CMAKE_SOURCE_DIR}/shaders"
    SHADERS_SPV_DIR="${SHADERS_OUTPUT}"
)

add_dependencies(${PROJECT_NAME} Shaders)