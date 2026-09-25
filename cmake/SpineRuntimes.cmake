include_guard(GLOBAL)

function(sl_spine_common target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/main/runtime_v2" "${PROJECT_SOURCE_DIR}/sdk/include")
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endfunction()

function(sl_spine_version target directory extension adapter prefix)
    file(GLOB runtime_sources CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/main/spine/${directory}/src/spine/*.${extension}")
    if(NOT runtime_sources)
        message(FATAL_ERROR "Missing source runtime: ${directory}")
    endif()
    add_library(${target} STATIC ${runtime_sources}
        "${PROJECT_SOURCE_DIR}/main/runtime_v2/${adapter}")
    sl_spine_common(${target})
    target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/main/spine/${directory}/include")
    target_compile_definitions(${target} PRIVATE ${ARGN})
    if(prefix)
        set(prefix_file "${PROJECT_SOURCE_DIR}/main/runtime_v2/cpp_symbol_prefixes/${prefix}")
        if(MSVC)
            target_compile_options(${target} PRIVATE "/FI${prefix_file}")
        else()
            target_compile_options(${target} PRIVATE -include "${prefix_file}")
        endif()
    endif()
    if(target STREQUAL "spinelove_spine_c_31")
        set_source_files_properties(${runtime_sources} PROPERTIES LANGUAGE CXX)
    endif()
    if(MSVC)
        if(prefix)
            target_compile_options(${target} PRIVATE /wd4010 /wd4828)
            if(directory MATCHES "^c-")
                target_compile_options(${target} PRIVATE /wd4244 /wd4267)
            endif()
        else()
            target_compile_options(${target} PRIVATE /wd4251 /wd4275)
        endif()
    endif()

endfunction()

sl_spine_version(spinelove_spine_c_31 c-3.1 c cpp_legacy_adapter/legacy_runtime_bridge.cpp spine_c31_prefix.h
    SL_RUNTIME_FACTORY_NAME=CreateCpp31Runtime SL_RUNTIME_DISPLAY_VERSION="3.1"
    SL_RUNTIME_KIND_VALUE=RuntimeKind::Cpp31 SL_SPINE_CPP_LEGACY_COLOR_FIELDS SL_SPINE_C_HAS_BONE_Y_DOWN
    SL_SPINE_C_REGION_COMPUTE_LEGACY SL_SPINE_C_MESH_COMPUTE_LEGACY SL_SPINE_C_MESH_DIRECT_VERTICES
    SL_SPINE_C_HAS_WEIGHTED_MESH SL_SPINE_C_NO_CLIPPING SL_SPINE_C_NO_BINARY)
sl_spine_version(spinelove_spine_c_34 c-3.4 c cpp_legacy_adapter/legacy_runtime_bridge.cpp spine_c34_prefix.h
    SL_RUNTIME_FACTORY_NAME=CreateCpp34Runtime SL_RUNTIME_DISPLAY_VERSION="3.4"
    SL_RUNTIME_KIND_VALUE=RuntimeKind::Cpp34 SL_SPINE_CPP_LEGACY_COLOR_FIELDS SL_SPINE_C_HAS_BONE_Y_DOWN
    SL_SPINE_C_REGION_COMPUTE_LEGACY SL_SPINE_C_MESH_COMPUTE_LEGACY SL_SPINE_C_MESH_WORLD_VERTICES_LENGTH
    SL_SPINE_C_NO_CLIPPING SL_SPINE_C_RESET_POSE_BEFORE_APPLY)
foreach(version 35 36 37)
    string(SUBSTRING "${version}" 1 1 minor)
    sl_spine_version(spinelove_spine_cpp_${version} cpp-3.${minor} cpp
        cpp_legacy_adapter/legacy_runtime_bridge.cpp spine_cpp${version}_prefix.h
        SL_RUNTIME_FACTORY_NAME=CreateCpp${version}Runtime SL_RUNTIME_DISPLAY_VERSION="3.${minor}"
        SL_RUNTIME_KIND_VALUE=RuntimeKind::Cpp${version})
endforeach()

sl_spine_version(spinelove_spine_cpp_38 cpp-3.8 cpp cpp_adapter/cpp_runtime_adapter.cpp ""
    spine=sl_spine38 indexOf=sl_spine38_atlasIndexOf SL_SPINE_NAMESPACE=sl_spine38
    SL_RUNTIME_FACTORY_NAME=CreateCpp38Runtime SL_RUNTIME_DISPLAY_VERSION="3.8" SL_SPINE_SKELETON_HAS_UPDATE)
sl_spine_version(spinelove_spine_cpp_40 cpp-4.0 cpp cpp_adapter/cpp_runtime_adapter.cpp ""
    spine=sl_spine40 indexOf=sl_spine40_atlasIndexOf SL_SPINE_NAMESPACE=sl_spine40
    SL_RUNTIME_FACTORY_NAME=CreateCpp40Runtime SL_RUNTIME_DISPLAY_VERSION="4.0"
    SL_SPINE_SKELETON_HAS_UPDATE SL_SPINE_ATLAS_PAGE_HAS_PMA)
sl_spine_version(spinelove_spine_cpp_41 cpp-4.1 cpp cpp_adapter/cpp_runtime_adapter.cpp ""
    spine=sl_spine41 indexOf=sl_spine41_atlasIndexOf SL_SPINE_NAMESPACE=sl_spine41
    SL_RUNTIME_FACTORY_NAME=CreateCpp41Runtime SL_RUNTIME_DISPLAY_VERSION="4.1"
    SL_SPINE_TEXTURE_REGION_API SL_SPINE_REGION_COMPUTE_USES_SLOT SL_SPINE_ATLAS_PAGE_HAS_TEXTURE SL_SPINE_ATLAS_PAGE_HAS_PMA)
sl_spine_version(spinelove_spine_cpp_42 cpp-4.2 cpp cpp_adapter/cpp_runtime_adapter.cpp ""
    spine=sl_spine42 indexOf=sl_spine42_atlasIndexOf SL_SPINE_NAMESPACE=sl_spine42
    SL_RUNTIME_FACTORY_NAME=CreateCpp42Runtime SL_RUNTIME_DISPLAY_VERSION="4.2"
    SL_SPINE_SKELETON_HAS_UPDATE SL_SPINE_TEXTURE_REGION_API SL_SPINE_REGION_COMPUTE_USES_SLOT
    SL_SPINE_ATLAS_PAGE_HAS_TEXTURE SL_SPINE_ATLAS_PAGE_HAS_PMA SL_SPINE_WORLD_TRANSFORM_HAS_PHYSICS)

add_library(spinelove_runtime_v2 STATIC
    "${PROJECT_SOURCE_DIR}/main/runtime_v2/cpp_runtime_manager.cpp"
    "${PROJECT_SOURCE_DIR}/main/runtime_v2/spine21_cpp/spine21_runtime.cpp")
sl_spine_common(spinelove_runtime_v2)
target_link_libraries(spinelove_runtime_v2 PUBLIC
    spinelove_spine_c_31 spinelove_spine_c_34
    spinelove_spine_cpp_35 spinelove_spine_cpp_36 spinelove_spine_cpp_37
    spinelove_spine_cpp_38 spinelove_spine_cpp_40 spinelove_spine_cpp_41 spinelove_spine_cpp_42)
add_library(SpineLove::Runtime ALIAS spinelove_runtime_v2)
