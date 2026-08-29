file(GLOB IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.*"
)
add_library(imgui STATIC
    ${IMGUI_SOURCES}
)
target_link_libraries(imgui
    PRIVATE
    Vulkan::Vulkan
    glfw
)
target_include_directories(imgui
    PUBLIC
    ${IMGUI_DIR}
)


