cmake_minimum_required(VERSION 3.20)
project(Mars-Wars-3026 VERSION 2.1.1.1.1 LANGUAGES CXX)

# ============================================================================
# Platform Detection & Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(PLATFORM_WINDOWS ON)
    message(STATUS "Configuring for Windows platform")
elseif(XBOX360)
    set(PLATFORM_XBOX360 ON)
    message(STATUS "Configuring for Xbox 360 platform")
elseif(XBOXONE)
    set(PLATFORM_XBOXONE ON)
    message(STATUS "Configuring for Xbox One platform")
endif()

# ============================================================================
# Build Options
# ============================================================================
option(TARGET_PLATFORM "Target platform: Windows|Xbox360|XboxOne" "Windows")
option(ENABLE_HLSL_COMPILATION "Compile HLSL shaders to CSO format" ON)
option(ENABLE_TRIANGULATION "Enable coordinate triangulation for eye-active vision" ON)
option(ENABLE_RENDER_TARGET_CONVERSION "Enable Xbox360->XboxOne render target conversion" OFF)
option(ENABLE_ISO_GRID_CAPTURE "Enable ISO grid image recording" ON)
option(ENABLE_MARS_DETONATION_RECORDING "Record Mars 3026 detonation sequence" ON)
option(CACHE_SHADERS "Cache compiled HLSL shaders" ON)
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy static analysis" OFF)
option(ENABLE_CPPCHECK "Enable CppCheck analysis" OFF)

# ============================================================================
# DirectX SDK Configuration
# ============================================================================
if(PLATFORM_WINDOWS OR PLATFORM_XBOX360 OR PLATFORM_XBOXONE)
    find_package(DirectX REQUIRED)
    include_directories(${DIRECTX_INCLUDE_DIR})
    link_directories(${DIRECTX_LIB_DIR})
endif()

# ============================================================================
# HLSL Shader Configuration
# ============================================================================
if(ENABLE_HLSL_COMPILATION)
    # FXC (Effects Compiler) for HLSL
    find_program(FXC_PATH fxc.exe HINTS "C:/Program Files (x86)/Windows Kits/10/bin/*/x64")
    
    if(NOT FXC_PATH)
        message(FATAL_ERROR "FXC.exe not found. Install Windows SDK for shader compilation.")
    endif()
    
    message(STATUS "Found FXC: ${FXC_PATH}")
    
    set(HLSL_SHADER_VERSION "5_0" CACHE STRING "HLSL Shader Model Version")
    set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/iso_grid
    ${CMAKE_SOURCE_DIR}/src/detonation
    ${CMAKE_BINARY_DIR}
)

# ============================================================================
# Source Files
# ============================================================================

# Core Engine
set(CORE_SOURCES
    src/core/engine.cpp
    src/core/engine.h
    src/core/platform_abstraction.cpp
    src/core/platform_abstraction.h
)

# Graphics Rendering
set(GRAPHICS_SOURCES
    src/graphics/render_device.cpp
    src/graphics/render_device.h
    src/graphics/render_target.cpp
    src/graphics/render_target.h
    src/graphics/shader_manager.cpp
    src/graphics/shader_manager.h
    src/graphics/triangulation.cpp
    src/graphics/triangulation.h
    src/graphics/eye_tracking.cpp
    src/graphics/eye_tracking.h
)

# UI Overlay System
set(UI_SOURCES
    src/ui/countdown_ui.cpp
    src/ui/countdown_ui.h
    src/ui/coordinate_overlay.cpp
    src/ui/coordinate_overlay.h
    src/ui/status_panel.cpp
    src/ui/status_panel.h
)

# ISO Grid System
set(ISO_GRID_SOURCES
    src/iso_grid/iso_grid_renderer.cpp
    src/iso_grid/iso_grid_renderer.h
    src/iso_grid/iso_grid_capture.cpp
    src/iso_grid/iso_grid_capture.h
    src/iso_grid/grid_serializer.cpp
    src/iso_grid/grid_serializer.h
)

# Mars Detonation Sequence
set(DETONATION_SOURCES
    src/detonation/detonation_sequence.cpp
    src/detonation/detonation_sequence.h
    src/detonation/timeline_controller.cpp
    src/detonation/timeline_controller.h
    src/detonation/cologny_location.cpp
    src/detonation/cologny_location.h
)

# HLSL Shaders
set(HLSL_SOURCES
    src/shaders/countdown.hlsl
    src/shaders/iso_grid.hlsl
    src/shaders/detonation_vfx.hlsl
    src/shaders/coordinate_display.hlsl
    src/shaders/render_target_convert.hlsl
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${GRAPHICS_SOURCES}
    ${UI_SOURCES}
    ${ISO_GRID_SOURCES}
    ${DETONATION_SOURCES}
    src/main.cpp
)

# ============================================================================
# HLSL Shader Compilation Function
# ============================================================================
function(compile_hlsl_shader SHADER_NAME PROFILE ENTRY_POINT)
    set(SHADER_PATH "${CMAKE_SOURCE_DIR}/src/shaders/${SHADER_NAME}.hlsl")
    set(OUTPUT_PATH "${SHADER_OUTPUT_DIR}/${SHADER_NAME}_${PROFILE}.cso")
    
    if(EXISTS "${SHADER_PATH}")
        add_custom_command(
            OUTPUT ${OUTPUT_PATH}
            COMMAND ${FXC_PATH}
                /T ${PROFILE}
                /E ${ENTRY_POINT}
                /Fo ${OUTPUT_PATH}
                /O3
                /Zi
                ${SHADER_PATH}
            MAIN_DEPENDENCY ${SHADER_PATH}
            COMMENT "Compiling HLSL shader: ${SHADER_NAME} (${PROFILE})"
            VERBATIM
        )
        list(APPEND SHADER_OUTPUTS ${OUTPUT_PATH})
        set(SHADER_OUTPUTS ${SHADER_OUTPUTS} PARENT_SCOPE)
    endif()
endfunction()

# Compile vertex and pixel shaders
if(ENABLE_HLSL_COMPILATION)
    compile_hlsl_shader(countdown vs_${HLSL_SHADER_VERSION} main_vs)
    compile_hlsl_shader(countdown ps_${HLSL_SHADER_VERSION} main_ps)
    compile_hlsl_shader(iso_grid vs_${HLSL_SHADER_VERSION} main_vs)
    compile_hlsl_shader(iso_grid ps_${HLSL_SHADER_VERSION} main_ps)
    compile_hlsl_shader(detonation_vfx vs_${HLSL_SHADER_VERSION} main_vs)
    compile_hlsl_shader(detonation_vfx ps_${HLSL_SHADER_VERSION} main_ps)
    compile_hlsl_shader(coordinate_display ps_${HLSL_SHADER_VERSION} main_ps)
    compile_hlsl_shader(render_target_convert ps_${HLSL_SHADER_VERSION} main_ps)
endif()

# ============================================================================
# Main Executable
# ============================================================================
add_executable(Mars3026
    ${ALL_SOURCES}
    ${SHADER_OUTPUTS}
)

# ============================================================================
# Platform-Specific Configuration
# ============================================================================
if(TARGET_PLATFORM STREQUAL "Xbox360")
    target_compile_definitions(Mars3026 PRIVATE
        PLATFORM_XBOX360=1
        ENABLE_TRIANGULATION=1
        ENABLE_EYE_TRACKING=1
    )
    target_link_libraries(Mars3026 PRIVATE
        d3d9.lib
        d3dx9.lib
        xbdm.lib
    )
elseif(TARGET_PLATFORM STREQUAL "XboxOne")
    target_compile_definitions(Mars3026 PRIVATE
        PLATFORM_XBOXONE=1
        ENABLE_TRIANGULATION=1
        ENABLE_EYE_TRACKING=1
        ENABLE_RENDER_TARGET_CONVERSION=1
        ENABLE_ISO_GRID_CAPTURE=1
    )
    target_link_libraries(Mars3026 PRIVATE
        d3d11.lib
        d3dx11.lib
        xbdm.lib
    )
else() # Windows
    target_compile_definitions(Mars3026 PRIVATE
        PLATFORM_WINDOWS=1
        ENABLE_TRIANGULATION=1
        ENABLE_ISO_GRID_CAPTURE=1
        ENABLE_MARS_DETONATION_RECORDING=1
    )
    target_link_libraries(Mars3026 PRIVATE
        d3d11.lib
        d3dcompiler.lib
        dxgi.lib
        dxguid.lib
    )
endif()

# ============================================================================
# Platform-Specific Source Files
# ============================================================================
if(TARGET_PLATFORM STREQUAL "Xbox360")
    target_sources(Mars3026 PRIVATE
        src/platform/xbox360/xbox360_platform.cpp
        src/platform/xbox360/xbox360_render.cpp
    )
elseif(TARGET_PLATFORM STREQUAL "XboxOne")
    target_sources(Mars3026 PRIVATE
        src/platform/xboxone/xboxone_platform.cpp
        src/platform/xboxone/xboxone_render.cpp
        src/platform/xboxone/render_target_converter.cpp
    )
else()
    target_sources(Mars3026 PRIVATE
        src/platform/windows/windows_platform.cpp
        src/platform/windows/windows_render.cpp
    )
endif()

# ============================================================================
# Render Target Conversion (Xbox360 -> XboxOne)
# ============================================================================
if(ENABLE_RENDER_TARGET_CONVERSION AND TARGET_PLATFORM STREQUAL "XboxOne")
    add_executable(RenderTargetConverter
        src/tools/render_target_converter.cpp
        src/graphics/render_target.cpp
        src/platform/xboxone/render_target_converter.cpp
    )
    target_link_libraries(RenderTargetConverter PRIVATE d3d11.lib dxgi.lib)
endif()

# ============================================================================
# ISO Grid Recorder Tool
# ============================================================================
if(ENABLE_ISO_GRID_CAPTURE)
    add_executable(ISOGridRecorder
        src/tools/iso_grid_recorder_tool.cpp
        src/iso_grid/iso_grid_renderer.cpp
        src/iso_grid/iso_grid_capture.cpp
        src/iso_grid/grid_serializer.cpp
        src/graphics/render_device.cpp
    )
    target_link_libraries(ISOGridRecorder PRIVATE d3d11.lib dxgi.lib)
endif()

# ============================================================================
# Static Analysis
# ============================================================================
if(ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        -header-filter=.*;
        -checks=*;
    )
endif()

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK ${CPPCHECK})
    endif()
endif()

# ============================================================================
# Unit Tests
# ============================================================================
enable_testing()

add_executable(UnitTests
    tests/test_triangulation.cpp
    tests/test_iso_grid.cpp
    tests/test_detonation_sequence.cpp
    src/graphics/triangulation.cpp
    src/iso_grid/iso_grid_renderer.cpp
    src/detonation/detonation_sequence.cpp
)

target_link_libraries(UnitTests PRIVATE gtest gtest_main)
add_test(NAME UnitTests COMMAND UnitTests)

# ============================================================================
# Output Configuration
# ============================================================================
set_target_properties(Mars3026 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if(ENABLE_RENDER_TARGET_CONVERSION)
    set_target_properties(RenderTargetConverter PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    )
endif()

if(ENABLE_ISO_GRID_CAPTURE)
    set_target_properties(ISOGridRecorder PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    )
endif()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS Mars3026
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

if(ENABLE_RENDER_TARGET_CONVERSION)
    install(TARGETS RenderTargetConverter RUNTIME DESTINATION bin)
endif()

if(ENABLE_ISO_GRID_CAPTURE)
    install(TARGETS ISOGridRecorder RUNTIME DESTINATION bin)
endif()

install(DIRECTORY ${SHADER_OUTPUT_DIR}/ DESTINATION bin/shaders)

# ============================================================================
# Print Configuration Summary
# ============================================================================
message(STATUS "")
message(STATUS "====== Mars Wars 3026 Build Configuration ======")
message(STATUS "Target Platform: ${TARGET_PLATFORM}")
message(STATUS "HLSL Shader Model: ${HLSL_SHADER_VERSION}")
message(STATUS "HLSL Compilation: ${ENABLE_HLSL_COMPILATION}")
message(STATUS "Triangulation: ${ENABLE_TRIANGULATION}")
message(STATUS "Render Target Conversion: ${ENABLE_RENDER_TARGET_CONVERSION}")
message(STATUS "ISO Grid Capture: ${ENABLE_ISO_GRID_CAPTURE}")
message(STATUS "Mars Detonation Recording: ${ENABLE_MARS_DETONATION_RECORDING}")
message(STATUS "Shader Cache: ${CACHE_SHADERS}")
message(STATUS "==============================================")
message(STATUS "")
