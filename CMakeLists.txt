# ketsui - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(ketsui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(KETSUI_SOURCES
    src/main.cpp
    src/ketsui_app.cpp
)

if(WIN32)
    add_executable(ketsui WIN32 ${KETSUI_SOURCES})
else()
    add_executable(ketsui ${KETSUI_SOURCES})
endif()

rexglue_setup_target(ketsui GPU_PLUGINS xenos)

# Find librexruntime.so without LD_LIBRARY_PATH: the SDK puts it in its own output
# directory, next to rex_app.cpp's runtime, so the binary must point there.
if(UNIX AND NOT APPLE)
    set_property(TARGET ketsui APPEND PROPERTY INSTALL_RPATH "$<TARGET_FILE_DIR:rexruntime>")
endif()

# rex_app.cpp (compiled into the host) includes rex/ui headers, which need imgui
# and Vulkan headers. Only the include paths are taken here: rex::ui, rex::input
# and rex::audio are already exported by librexruntime, and linking their object
# libraries again duplicates the cvar registry and ImGui.
foreach(_rexglue_ui_dep IN ITEMS rex::ui imgui::imgui Vulkan::Headers GPUOpen::VulkanMemoryAllocator spirv-tools-headers)
    target_include_directories(ketsui PRIVATE $<TARGET_PROPERTY:${_rexglue_ui_dep},INTERFACE_INCLUDE_DIRECTORIES>)
endforeach()
