cmake_minimum_required(VERSION 3.20)
project(SpineLove LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(SL_DEPS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../outfile")
set(SL_DXLIB_DIR "${SL_DEPS_ROOT}/dxlib")
set(SL_IMGUI_DIR "${SL_DEPS_ROOT}/imgui")

if(MSVC)
    add_compile_options(/utf-8 /wd4828 /wd4010)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
    add_link_options("/pdbaltpath:%_PDB%")
endif()

file(GLOB SL_IMGUI_CORE   "${SL_IMGUI_DIR}/*.cpp" "${SL_IMGUI_DIR}/*.h")
file(GLOB SL_VIEWER_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SL_VIEWER_UI    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h")
file(GLOB SL_VIEWER_NATUI "${CMAKE_CURRENT_SOURCE_DIR}/sl-widgets/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/sl-widgets/*.h")

list(APPEND SL_IMGUI_BACKENDS
    "${SL_IMGUI_DIR}/backends/imgui_impl_win32.cpp" "${SL_IMGUI_DIR}/backends/imgui_impl_win32.h"
    "${SL_IMGUI_DIR}/backends/imgui_impl_dx11.cpp"  "${SL_IMGUI_DIR}/backends/imgui_impl_dx11.h"
    "${SL_IMGUI_DIR}/backends/imgui_impl_dx9.cpp"   "${SL_IMGUI_DIR}/backends/imgui_impl_dx9.h")

list(APPEND SL_RUNTIME_SRC
    "dxlib/shared/dll_entry.h"
    "dxlib/shared/dll_bridge.cpp" "dxlib/shared/dll_bridge.h"
    "dxlib/shared/file_checker.cpp" "dxlib/shared/file_checker.h")

add_executable(SpineLove WIN32
    ${SL_IMGUI_CORE} ${SL_IMGUI_BACKENDS}
    ${SL_VIEWER_SRC} ${SL_VIEWER_UI} ${SL_VIEWER_NATUI}
    ${SL_RUNTIME_SRC}
    "${CMAKE_CURRENT_SOURCE_DIR}/sl_hires_display.manifest"
    "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")

set_target_properties(SpineLove PROPERTIES OUTPUT_NAME "spine love")

target_include_directories(SpineLove PRIVATE ${SL_DXLIB_DIR} ${SL_IMGUI_DIR})
target_link_directories(SpineLove PRIVATE ${SL_DXLIB_DIR})
target_link_libraries(SpineLove PRIVATE comctl32)
target_compile_definitions(SpineLove PRIVATE UNICODE _UNICODE)

add_custom_command(TARGET SpineLove POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/lang" "$<TARGET_FILE_DIR:SpineLove>/lang"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/app.png" "$<TARGET_FILE_DIR:SpineLove>/app.png"
    COMMENT "Deploying assets to output directory")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/NotoSansSC-Regular.ttf")
    add_custom_command(TARGET SpineLove POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/NotoSansSC-Regular.ttf" "$<TARGET_FILE_DIR:SpineLove>/NotoSansSC-Regular.ttf"
        COMMENT "Deploying bundled font to output directory")
endif()
