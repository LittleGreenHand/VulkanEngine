set(KTX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/texture.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/hashlist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/checkheader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/swap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/memstream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/filestream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/vkloader.c
)
add_library(ktx STATIC
    ${KTX_SOURCES}
)

target_include_directories(ktx
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(ktx
    PUBLIC
    Vulkan::Vulkan
)