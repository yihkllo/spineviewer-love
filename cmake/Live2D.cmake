include_guard(GLOBAL)

set(cubism_root "${PROJECT_SOURCE_DIR}/third_party/live2d")
set(cubism_core "")
if(WIN32)
    if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The bundled Live2D Core requires the MSVC x64 ABI; provide a matching Core before using another Windows toolchain.")
    endif()
    set(cubism_core
        "$<$<CONFIG:Debug>:${cubism_root}/Core/lib/windows/x86_64/143/Live2DCubismCore_MDd.lib>"
        "$<$<NOT:$<CONFIG:Debug>>:${cubism_root}/Core/lib/windows/x86_64/143/Live2DCubismCore_MD.lib>")
else()
    if(ANDROID)
        set(cubism_candidate "${cubism_root}/Core/lib/android/${ANDROID_ABI}/libLive2DCubismCore.a")
    elseif(IOS)
        set(cubism_candidate "${cubism_root}/Core/lib/ios/${CMAKE_OSX_SYSROOT}/libLive2DCubismCore.a")
    elseif(APPLE)
        set(cubism_candidate "${cubism_root}/Core/lib/macos/libLive2DCubismCore.a")
    else()
        set(cubism_candidate "${cubism_root}/Core/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/libLive2DCubismCore.a")
    endif()
    if(EXISTS "${cubism_candidate}")
        set(cubism_core "${cubism_candidate}")
    endif()
endif()

if(NOT cubism_core)
    message(STATUS "Live2D Cubism Core was not found for this platform; Live2D support is disabled.")
    return()
endif()

file(GLOB cubism_sources CONFIGURE_DEPENDS "${cubism_root}/Framework/src/*.cpp")
set(cubism_directories Effect Id Math Model Motion Physics Rendering Type Utils)
if(WIN32)
    list(APPEND cubism_directories Rendering/D3D11)
endif()
foreach(directory ${cubism_directories})
    file(GLOB group CONFIGURE_DEPENDS "${cubism_root}/Framework/src/${directory}/*.cpp")
    list(APPEND cubism_sources ${group})
endforeach()

add_library(spinelove_cubism STATIC ${cubism_sources})
if(WIN32)
    target_sources(spinelove_cubism PRIVATE
        "${PROJECT_SOURCE_DIR}/main/render_d3d11/d3d11_renderer.cpp"
        "${PROJECT_SOURCE_DIR}/main/render_d3d11/d3d11_texture.cpp")
endif()
target_compile_features(spinelove_cubism PUBLIC cxx_std_17)
target_include_directories(spinelove_cubism
    PUBLIC "${cubism_root}/Framework/src" "${cubism_root}/Core/include"
    PRIVATE "${PROJECT_SOURCE_DIR}/main" "${PROJECT_SOURCE_DIR}/sdk/include" "${PROJECT_SOURCE_DIR}/third_party/stb")
target_link_libraries(spinelove_cubism PUBLIC ${cubism_core})
if(MSVC)
    target_compile_options(spinelove_cubism PRIVATE /utf-8)
    target_compile_definitions(spinelove_cubism PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
endif()
if(WIN32)
    target_link_libraries(spinelove_cubism PUBLIC d3d11 dxgi d3dcompiler windowscodecs ole32)
endif()
set_target_properties(spinelove_cubism PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(spinelove_live2d STATIC
    "${PROJECT_SOURCE_DIR}/main/live2d/live2d_module.cpp"
    "${PROJECT_SOURCE_DIR}/main/live2d/live2d_rhi_renderer.cpp"
    "${PROJECT_SOURCE_DIR}/main/common/module_audio.cpp"
    "${PROJECT_SOURCE_DIR}/main/common/sl_text_codec.cpp")
target_compile_features(spinelove_live2d PUBLIC cxx_std_17)
target_compile_definitions(spinelove_live2d PUBLIC SPINELOVE_HAS_LIVE2D=1 $<$<BOOL:${WIN32}>:SL_LIVE2D_D3D11=1>)
if(MSVC)
    target_compile_options(spinelove_live2d PRIVATE /utf-8)
    target_compile_definitions(spinelove_live2d PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
endif()
target_include_directories(spinelove_live2d PRIVATE
    "${PROJECT_SOURCE_DIR}/main" "${PROJECT_SOURCE_DIR}/sdk/include" "${PROJECT_SOURCE_DIR}/third_party/stb")
target_link_libraries(spinelove_live2d PRIVATE nlohmann_json Qt6::GuiPrivate)
target_link_libraries(spinelove_live2d PUBLIC spinelove_cubism Qt6::Gui Qt6::Multimedia)
set_target_properties(spinelove_live2d PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(WIN32)
    add_library(spinelove_live2d_windows ALIAS spinelove_live2d)
endif()
