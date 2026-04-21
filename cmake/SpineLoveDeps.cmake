include_guard(GLOBAL)

get_filename_component(SLOVE_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(SLOVE_VENDOR_ROOT "${SLOVE_PROJECT_ROOT}/outfile" CACHE PATH "Directory used to store prepared third-party dependencies")
set(SLOVE_BOOTSTRAP_ROOT "${CMAKE_BINARY_DIR}/slove-bootstrap" CACHE PATH "Temporary directory for dependency archives and extraction")
option(SLOVE_CLEAN_BOOTSTRAP_AFTER_PREPARE "Delete temporary dependency bootstrap files after preparation" ON)
set(SLOVE_DOWNLOAD_CHANNEL "official" CACHE STRING "Dependency source channel: official, custom, or offline")
set_property(CACHE SLOVE_DOWNLOAD_CHANNEL PROPERTY STRINGS official custom offline)
set(SLOVE_DOWNLOAD_RETRY_COUNT "3" CACHE STRING "How many times dependency downloads should be retried before failing")
set(SLOVE_DOWNLOAD_TIMEOUT_SECONDS "600" CACHE STRING "Per-attempt dependency download timeout in seconds")
set(SLOVE_DOWNLOAD_INACTIVITY_TIMEOUT_SECONDS "120" CACHE STRING "Per-attempt inactivity timeout in seconds")

set(SLOVE_DXLIB_ARCHIVE_URL "https://dxlib.xsrv.jp/temp/DxLibVCTest.zip" CACHE STRING "Archive URL for DxLib package")
set(SLOVE_IMGUI_VERSION "1.92.6-docking" CACHE STRING "Dear ImGui docking tag to install")
set(SLOVE_IMGUI_ARCHIVE_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${SLOVE_IMGUI_VERSION}.zip" CACHE STRING "Archive URL for Dear ImGui package")

set(SLOVE_CUSTOM_DXLIB_ARCHIVE_URL "" CACHE STRING "Optional custom archive URL for DxLib")
set(SLOVE_CUSTOM_IMGUI_ARCHIVE_URL "" CACHE STRING "Optional custom archive URL for Dear ImGui")

function(slove_prepare_dependencies)
    file(MAKE_DIRECTORY "${SLOVE_VENDOR_ROOT}")
    file(MAKE_DIRECTORY "${SLOVE_BOOTSTRAP_ROOT}")

    slove_prepare_dxlib()
    slove_prepare_spine_runtimes()
    slove_patch_spine35_runtime()
    slove_prepare_imgui()

    if(SLOVE_CLEAN_BOOTSTRAP_AFTER_PREPARE AND EXISTS "${SLOVE_BOOTSTRAP_ROOT}")
        file(REMOVE_RECURSE "${SLOVE_BOOTSTRAP_ROOT}")
    endif()
endfunction()

function(slove_prepare_dxlib)
    set(dxlib_dir "${SLOVE_VENDOR_ROOT}/dxlib")
    if(EXISTS "${dxlib_dir}")
        return()
    endif()

    slove_choose_url(resolved_url DXLIB "${SLOVE_DXLIB_ARCHIVE_URL}")
    slove_fetch_archive("DxLibVCTest" "${resolved_url}" extracted_dir)

    file(GLOB dxlib_libs "${extracted_dir}/DxLibVCTest/*.lib")
    file(GLOB dxlib_headers "${extracted_dir}/DxLibVCTest/*.h")

    foreach(lib IN LISTS dxlib_libs)
        if(lib MATCHES "_vs2015_x64_MD\.lib$" OR lib MATCHES "_vs2015_x64_MDd\.lib$")
            file(COPY "${lib}" DESTINATION "${dxlib_dir}")
        endif()
    endforeach()

    foreach(header IN LISTS dxlib_headers)
        file(COPY "${header}" DESTINATION "${dxlib_dir}")
    endforeach()
endfunction()

function(slove_prepare_imgui)
    set(imgui_dir "${SLOVE_VENDOR_ROOT}/imgui")
    if(EXISTS "${imgui_dir}")
        return()
    endif()

    slove_choose_url(resolved_url IMGUI "${SLOVE_IMGUI_ARCHIVE_URL}")
    set(archive_key "imgui-${SLOVE_IMGUI_VERSION}")
    slove_fetch_archive("${archive_key}" "${resolved_url}" extracted_dir)
    file(COPY "${extracted_dir}/imgui-${SLOVE_IMGUI_VERSION}/" DESTINATION "${imgui_dir}")
endfunction()

function(slove_prepare_spine_runtimes)
    set(local_runtime_root "${SLOVE_PROJECT_ROOT}/main/spine")
    set(required_runtime_dirs
        "c-2.1"
        "c-3.4"
        "c-3.5"
        "c-3.6"
        "c-3.7"
        "cpp-3.8"
        "cpp-4.0"
        "cpp-4.1"
        "cpp-4.2"
    )

    foreach(runtime_name IN LISTS required_runtime_dirs)
        set(runtime_dir "${local_runtime_root}/${runtime_name}")
        if(NOT EXISTS "${runtime_dir}/src" OR NOT EXISTS "${runtime_dir}/include")
            message(FATAL_ERROR "[spinelove] missing local Spine runtime: ${runtime_name} under ${local_runtime_root}")
        endif()
    endforeach()
endfunction()

function(slove_patch_spine35_runtime)
    set(spine35_dir "${SLOVE_PROJECT_ROOT}/main/spine/c-3.5")
    if(NOT EXISTS "${spine35_dir}/src/spine/extension.c")
        return()
    endif()

    set(target_files
        "${spine35_dir}/src/spine/extension.c"
        "${spine35_dir}/include/spine/extension.h"
    )

    foreach(target_file IN LISTS target_files)
        slove_patch_file("${target_file}" "_malloc" "_spMalloc")
        slove_patch_file("${target_file}" "_calloc" "_spCalloc")
        slove_patch_file("${target_file}" "_realloc" "_spRealloc")
        slove_patch_file("${target_file}" "_free" "_spFree")
        slove_patch_file("${target_file}" "char* _readFile" "char* _spReadFile")
    endforeach()
endfunction()

function(slove_patch_file file_path old_text new_text)
    if(NOT EXISTS "${file_path}")
        return()
    endif()

    file(READ "${file_path}" file_content)
    string(REPLACE "${old_text}" "${new_text}" file_content "${file_content}")
    file(WRITE "${file_path}" "${file_content}")
endfunction()

function(slove_choose_url out_var kind default_url)
    set(selected_url "${default_url}")

    if(SLOVE_DOWNLOAD_CHANNEL STREQUAL "custom")
        if(kind STREQUAL "DXLIB" AND NOT SLOVE_CUSTOM_DXLIB_ARCHIVE_URL STREQUAL "")
            set(selected_url "${SLOVE_CUSTOM_DXLIB_ARCHIVE_URL}")
        elseif(kind STREQUAL "IMGUI" AND NOT SLOVE_CUSTOM_IMGUI_ARCHIVE_URL STREQUAL "")
            set(selected_url "${SLOVE_CUSTOM_IMGUI_ARCHIVE_URL}")
        endif()
    elseif(SLOVE_DOWNLOAD_CHANNEL STREQUAL "offline")
        set(selected_url "")
    endif()

    set(${out_var} "${selected_url}" PARENT_SCOPE)
endfunction()

function(slove_download_with_retries archive_path archive_name)
    set(url_candidates ${ARGN})
    foreach(candidate_url IN LISTS url_candidates)
        if(candidate_url STREQUAL "")
            continue()
        endif()

        math(EXPR retry_last "${SLOVE_DOWNLOAD_RETRY_COUNT} - 1")
        foreach(retry_index RANGE ${retry_last})
            math(EXPR attempt_number "${retry_index} + 1")
            message(STATUS "[spinelove] downloading ${archive_name} (attempt ${attempt_number}/${SLOVE_DOWNLOAD_RETRY_COUNT}) from ${candidate_url}")
            file(DOWNLOAD "${candidate_url}" "${archive_path}"
                STATUS download_status
                TLS_VERIFY ON
                TIMEOUT ${SLOVE_DOWNLOAD_TIMEOUT_SECONDS}
                INACTIVITY_TIMEOUT ${SLOVE_DOWNLOAD_INACTIVITY_TIMEOUT_SECONDS})
            list(GET download_status 0 download_code)
            if(download_code EQUAL 0)
                if(EXISTS "${archive_path}")
                    file(SIZE "${archive_path}" archive_size)
                    if(archive_size GREATER 0)
                        return()
                    endif()
                endif()
                message(WARNING "[spinelove] download produced an empty archive for ${archive_name}; retrying")
            endif()
            list(GET download_status 1 download_message)
            message(WARNING "[spinelove] download attempt failed for ${archive_name}: ${download_message}")
            if(EXISTS "${archive_path}")
                file(REMOVE "${archive_path}")
            endif()
        endforeach()
    endforeach()

    message(FATAL_ERROR "[spinelove] failed to download ${archive_name} after trying all configured sources")
endfunction()

function(slove_fetch_archive archive_name archive_urls out_dir_var)
    set(archive_path "${SLOVE_BOOTSTRAP_ROOT}/${archive_name}.zip")
    set(extracted_dir "${SLOVE_BOOTSTRAP_ROOT}/${archive_name}")

    if(NOT EXISTS "${archive_path}")
        if("${archive_urls}" STREQUAL "")
            message(FATAL_ERROR "[spinelove] offline channel selected, but archive is missing: ${archive_name}")
        endif()
        slove_download_with_retries("${archive_path}" "${archive_name}" ${archive_urls})
    endif()

    if(NOT EXISTS "${archive_path}")
        message(FATAL_ERROR "[spinelove] archive is missing after download: ${archive_name}")
    endif()

    file(SIZE "${archive_path}" archive_size)
    if(archive_size LESS_EQUAL 0)
        file(REMOVE "${archive_path}")
        message(FATAL_ERROR "[spinelove] archive is empty after download: ${archive_name}")
    endif()

    if(EXISTS "${extracted_dir}")
        file(REMOVE_RECURSE "${extracted_dir}")
    endif()
    message(STATUS "[spinelove] extracting ${archive_name}")
    file(MAKE_DIRECTORY "${extracted_dir}")
    file(ARCHIVE_EXTRACT INPUT "${archive_path}" DESTINATION "${extracted_dir}")

    set(${out_dir_var} "${extracted_dir}" PARENT_SCOPE)
endfunction()
