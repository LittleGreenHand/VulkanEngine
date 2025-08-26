# Copyright (c) 2016-2025, Sascha Willems
# SPDX-License-Identifier: MIT

file(GLOB BASE_SRC "*.cpp" "*.hpp" "*.h" "${CMAKE_SOURCE_DIR}/external/imgui/*.cpp")

set(KTX_DIR ${CMAKE_SOURCE_DIR}/external/ktx)
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c
    ${KTX_DIR}/lib/vkloader.c
)

add_library(base STATIC ${BASE_SRC} ${KTX_SOURCES})
target_link_libraries(base PUBLIC Vulkan::Vulkan ${WINLIBS})
