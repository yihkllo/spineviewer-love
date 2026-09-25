cmake_minimum_required(VERSION 3.22)
project(SpineLoveEX VERSION 0.1.0 LANGUAGES C CXX)
if(MSVC AND DEFINED SPINELOVE_MSVC_INCLUDE_PREFIX)
    set(CMAKE_CL_SHOWINCLUDES_PREFIX "${SPINELOVE_MSVC_INCLUDE_PREFIX}")
endif()

if(WIN32)
    add_executable(SpineLoveEX WIN32 main/launcher/launcher.cpp main/resources/app.rc)
    target_compile_features(SpineLoveEX PRIVATE cxx_std_17)
    target_compile_definitions(SpineLoveEX PRIVATE UNICODE _UNICODE)
    target_link_libraries(SpineLoveEX PRIVATE shell32)
    if(MSVC)
        target_compile_options(SpineLoveEX PRIVATE /utf-8)
        set_property(TARGET SpineLoveEX PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(MSVC)
    file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}" SPINELOVE_SOURCE_MAP_FROM)
    add_compile_options(/experimental:deterministic "/pathmap:${SPINELOVE_SOURCE_MAP_FROM}=spinelove")
endif()

option(SPINELOVE_BUILD_QT_APP "Build the Qt application, its SDK and plugins" OFF)
option(SPINELOVE_BUILD_RUNTIME_TESTS "Build the tests (runtime checks; application and QML tests with the Qt app)" ON)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE "${PROJECT_SOURCE_DIR}/third_party/nlohmann/include")
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

include(cmake/SpineRuntimes.cmake)

if(SPINELOVE_BUILD_RUNTIME_TESTS)
    enable_testing()
endif()

if(SPINELOVE_BUILD_QT_APP)
    add_subdirectory(main/qt)
endif()

if(SPINELOVE_BUILD_RUNTIME_TESTS)
    add_subdirectory(tests)
endif()
