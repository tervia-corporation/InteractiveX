cmake_minimum_required(VERSION 3.20)
project(InteractiveX LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IX_BUILD_ENGINE "Build InteractiveX engine library" ON)
option(IX_BUILD_DEBUG "Build InteractiveX debug app" OFF)
option(IX_BUILD_EDITOR "Build InteractiveX editor app" OFF)
option(IX_BUILD_ALL "Build engine + debug + editor" OFF)

if(IX_BUILD_ALL)
    set(IX_BUILD_ENGINE ON)
    set(IX_BUILD_DEBUG ON)
    set(IX_BUILD_EDITOR ON)
endif()

# Perfis:
# Windows: DX9, DX10, DX11, DX12, VULKAN, OPENGL31, OPENGL, ALL
# Linux:   VULKAN, OPENGL31, OPENGL, ALL
set(IX_RENDERER_PROFILE "OPENGL" CACHE STRING "Renderer profile preset")
set_property(CACHE IX_RENDERER_PROFILE PROPERTY STRINGS DX9 DX10 DX11 DX12 VULKAN OPENGL31 OPENGL ALL)
string(TOUPPER "${IX_RENDERER_PROFILE}" IX_RENDERER_PROFILE_UPPER)

option(IX_ENABLE_DX9 "Enable DirectX 9 backend" OFF)
option(IX_ENABLE_DX10 "Enable DirectX 10 backend" OFF)
option(IX_ENABLE_DX11 "Enable DirectX 11 backend" OFF)
option(IX_ENABLE_DX12 "Enable DirectX 12 backend" OFF)
option(IX_ENABLE_VULKAN "Enable Vulkan backend" OFF)
option(IX_ENABLE_OPENGL31 "Enable OpenGL 3.1 legacy backend" OFF)
option(IX_ENABLE_OPENGL43 "Enable OpenGL 4.3 backend" OFF)

set(IX_IS_WINDOWS FALSE)
set(IX_IS_LINUX FALSE)
if(WIN32)
    set(IX_IS_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(IX_IS_LINUX TRUE)
endif()

if(IX_RENDERER_PROFILE_UPPER STREQUAL "DX9")
    set(IX_ENABLE_DX9 ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "DX10")
    set(IX_ENABLE_DX10 ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "DX11")
    set(IX_ENABLE_DX11 ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "DX12")
    set(IX_ENABLE_DX12 ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "VULKAN")
    set(IX_ENABLE_VULKAN ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "OPENGL31")
    set(IX_ENABLE_OPENGL31 ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "OPENGL")
    set(IX_ENABLE_OPENGL43 ON)
elseif(IX_RENDERER_PROFILE_UPPER STREQUAL "ALL")
    set(IX_ENABLE_DX9 ON)
    set(IX_ENABLE_DX10 ON)
    set(IX_ENABLE_DX11 ON)
    set(IX_ENABLE_DX12 ON)
    set(IX_ENABLE_VULKAN ON)
    set(IX_ENABLE_OPENGL31 ON)
    set(IX_ENABLE_OPENGL43 ON)
else()
    message(FATAL_ERROR "IX_RENDERER_PROFILE must be DX9, DX10, DX11, DX12, VULKAN, OPENGL31, OPENGL or ALL")
endif()

if(NOT IX_IS_WINDOWS)
    if(IX_ENABLE_DX9 OR IX_ENABLE_DX10 OR IX_ENABLE_DX11 OR IX_ENABLE_DX12)
        if(IX_RENDERER_PROFILE_UPPER STREQUAL "ALL")
            message(STATUS "Non-Windows platform detected: disabling DirectX backends from ALL profile.")
            set(IX_ENABLE_DX9 OFF)
            set(IX_ENABLE_DX10 OFF)
            set(IX_ENABLE_DX11 OFF)
            set(IX_ENABLE_DX12 OFF)
        else()
            message(FATAL_ERROR "DirectX backends are Windows-only. On Linux use VULKAN/OPENGL31/OPENGL/ALL.")
        endif()
    endif()
endif()

set(IX_SOURCES
    engine/huhlu/huhlu_lighting.cpp
    engine/huhlu/huhlu_fbx.cpp
    engine/huhlu/huhlu_fire3d.cpp
    engine/huhlu/huhlu_sky.cpp
    engine/huhlu/ix_material.cpp
    engine/huhlu/huhlu_camera.cpp
    engine/huhlu/huhlu_scene_ixs.cpp
    engine/huhlu/huhlu_gltf.cpp
    engine/huhlu/huhlu_hierarchy.cpp
    engine/huhlu/huhlu_obj.cpp
    engine/huhlu/huhlu_postfx.cpp
    engine/huhlu/huhlu_physics.cpp
    engine/huhlu/huhlu_particles.cpp
)

if(IX_ENABLE_DX9)
    list(APPEND IX_SOURCES engine/renderer.cpp engine/renderer_dx9.cpp)
endif()
if(IX_ENABLE_DX10)
    list(APPEND IX_SOURCES engine/renderer_dx10.cpp)
endif()
if(IX_ENABLE_DX11)
    list(APPEND IX_SOURCES engine/renderer_dx11.cpp)
endif()
if(IX_ENABLE_DX12)
    list(APPEND IX_SOURCES engine/renderer_dx12.cpp)
endif()
if(IX_ENABLE_VULKAN)
    list(APPEND IX_SOURCES engine/renderer_vulkan.cpp)
endif()
if(IX_ENABLE_OPENGL31)
    list(APPEND IX_SOURCES engine/renderer_gl31_legacy.cpp)
endif()
if(IX_ENABLE_OPENGL43)
    list(APPEND IX_SOURCES engine/renderer_gl43.cpp)
endif()

if(NOT IX_SOURCES)
    message(FATAL_ERROR "No renderer backend selected")
endif()

if(IX_ENABLE_DX9)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_dx9.cpp)
elseif(IX_ENABLE_DX10)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_dx10.cpp)
elseif(IX_ENABLE_DX11)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_dx11.cpp)
elseif(IX_ENABLE_DX12)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_dx12.cpp)
elseif(IX_ENABLE_VULKAN)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_vulkan.cpp)
elseif(IX_ENABLE_OPENGL43)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_gl43.cpp)
elseif(IX_ENABLE_OPENGL31)
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_gl31.cpp)
else()
    list(APPEND IX_SOURCES engine/huhlu/huhlu_backend_stub.cpp)
endif()

add_library(interactivex_render_config INTERFACE)
target_compile_definitions(interactivex_render_config INTERFACE
    IX_PLATFORM_WINDOWS=$<BOOL:${IX_IS_WINDOWS}>
    IX_PLATFORM_LINUX=$<BOOL:${IX_IS_LINUX}>
    IX_ENABLE_DX9=$<BOOL:${IX_ENABLE_DX9}>
    IX_ENABLE_DX10=$<BOOL:${IX_ENABLE_DX10}>
    IX_ENABLE_DX11=$<BOOL:${IX_ENABLE_DX11}>
    IX_ENABLE_DX12=$<BOOL:${IX_ENABLE_DX12}>
    IX_ENABLE_VULKAN=$<BOOL:${IX_ENABLE_VULKAN}>
    IX_ENABLE_OPENGL31=$<BOOL:${IX_ENABLE_OPENGL31}>
    IX_ENABLE_OPENGL43=$<BOOL:${IX_ENABLE_OPENGL43}>
)

if(IX_BUILD_ENGINE OR IX_BUILD_DEBUG OR IX_BUILD_EDITOR)
    add_library(interactivex_engine STATIC ${IX_SOURCES})
    target_include_directories(interactivex_engine PUBLIC engine)
    target_link_libraries(interactivex_engine PUBLIC interactivex_render_config)
endif()

if(IX_BUILD_DEBUG)
    add_executable(interactivex_debug engine/debug/debug_scenes.cpp)
    target_link_libraries(interactivex_debug PRIVATE interactivex_engine)
endif()

if(IX_BUILD_EDITOR)
    add_executable(interactivex_editor
        engine/editor/editor.cpp
        engine/editor/editor_main.cpp
    )
    target_link_libraries(interactivex_editor PRIVATE interactivex_engine)
endif()

message(STATUS "InteractiveX renderer profile: ${IX_RENDERER_PROFILE_UPPER}")
message(STATUS "Build toggles: engine=${IX_BUILD_ENGINE}, debug=${IX_BUILD_DEBUG}, editor=${IX_BUILD_EDITOR}, all=${IX_BUILD_ALL}")
message(STATUS "Platform: windows=${IX_IS_WINDOWS}, linux=${IX_IS_LINUX}")
message(STATUS "  IX_ENABLE_DX9=${IX_ENABLE_DX9}")
message(STATUS "  IX_ENABLE_DX10=${IX_ENABLE_DX10}")
message(STATUS "  IX_ENABLE_DX11=${IX_ENABLE_DX11}")
message(STATUS "  IX_ENABLE_DX12=${IX_ENABLE_DX12}")
message(STATUS "  IX_ENABLE_VULKAN=${IX_ENABLE_VULKAN}")
message(STATUS "  IX_ENABLE_OPENGL31=${IX_ENABLE_OPENGL31}")
message(STATUS "  IX_ENABLE_OPENGL43=${IX_ENABLE_OPENGL43}")
