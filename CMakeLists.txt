cmake_minimum_required(VERSION 3.14)
project(Rift_Renderer_SimpleRenderer VERSION 0.1.1)

add_library(
        Rift_Renderer_SimpleRenderer
        STATIC
        private/Engine/Renderer/SimpleRenderer.cpp
)

target_include_directories(
        Rift_Renderer_SimpleRenderer
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/public"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private"
)

rift_resolve_module_libs("Rift.Core.Runtime" RIFT_SIMPLE_RENDERER_DEPS)

target_link_libraries(Rift_Renderer_SimpleRenderer ${RIFT_SIMPLE_RENDERER_DEPS})

rift_bundle_folder(${CMAKE_CURRENT_SOURCE_DIR}/assets)