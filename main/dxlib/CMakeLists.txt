cmake_minimum_required(VERSION 3.20)
project(SpineRuntime-dll)

set(RT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}")
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../outfile")
set(LOCAL_SPINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../spine")
set(VENDORED_SPINE_DIR "${DEPS_DIR}/spine")
set(DXLIB_DIR "${DEPS_DIR}/dxlib")

set(OUTPUT_BIN_DIR     "${CMAKE_BINARY_DIR}/bin/spinedll")
set(OUTPUT_ARCHIVE_DIR "${CMAKE_BINARY_DIR}/lib")

set(SPINE_VERSIONS_C   2.1 3.4 3.5 3.6 3.7)
set(SPINE_VERSIONS_CPP 3.8 4.0 4.1 4.2)

function(add_spine_target RUNTIME_LANGUAGE RUNTIME_VERSION)
	string(REPLACE "." "" VER_NODOT "${RUNTIME_VERSION}")

	if(RUNTIME_LANGUAGE STREQUAL "C")
		set(TARGET_NAME       "SpinePlayerC_${VER_NODOT}")
		set(TARGET_FILENAME   "SpineC${VER_NODOT}")
		set(BACKEND_DIR       "${RT_DIR}/backend_c")
		set(SPINE_SRC_EXT     "c")
		set(SPINE_RUNTIME_SUFFIX "c-${RUNTIME_VERSION}")
	else()
		set(TARGET_NAME       "SpinePlayerCpp_${VER_NODOT}")
		set(TARGET_FILENAME   "SpineCpp${VER_NODOT}")
		set(BACKEND_DIR       "${RT_DIR}/backend_cpp")
		set(SPINE_SRC_EXT     "cpp")
		set(SPINE_RUNTIME_SUFFIX "cpp-${RUNTIME_VERSION}")
	endif()

	set(SPINE_RUNTIME_DIR "${LOCAL_SPINE_DIR}/${SPINE_RUNTIME_SUFFIX}")
	if(NOT EXISTS "${SPINE_RUNTIME_DIR}/src")
		set(SPINE_RUNTIME_DIR "${VENDORED_SPINE_DIR}/${SPINE_RUNTIME_SUFFIX}")
	endif()

	add_library(${TARGET_NAME} SHARED
		${RT_DIR}/shared/dll_entry.cpp
		${RT_DIR}/shared/dll_entry.h
		${RT_DIR}/shared/dll_bridge.cpp
		${RT_DIR}/shared/dll_bridge.h
	)

	set_target_properties(${TARGET_NAME} PROPERTIES
		OUTPUT_NAME              "${TARGET_FILENAME}"
		RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}"
		ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ARCHIVE_DIR}"
	)

	if(RUNTIME_LANGUAGE STREQUAL "C" AND RUNTIME_VERSION EQUAL 2.1)
		file(GLOB BACKEND_SRC "${BACKEND_DIR}/*.cpp")
		list(REMOVE_ITEM BACKEND_SRC "${BACKEND_DIR}/drawable.cpp")

		file(GLOB BASE_SPINE_SRC "${SPINE_RUNTIME_DIR}/src/spine/*.c")

		target_sources(${TARGET_NAME} PRIVATE
			${BACKEND_DIR}/Spine21/dxlib_spine_c_21.cpp
			${BACKEND_DIR}/Spine21/sp21_skeleton_binary.c
			${BASE_SPINE_SRC}
			${BACKEND_SRC}
		)

		target_include_directories(${TARGET_NAME} BEFORE PRIVATE
			${BACKEND_DIR}/Spine21
			${SPINE_RUNTIME_DIR}/include
		)

		set(SPINE_RUNTIME_DIR "")

	elseif(RUNTIME_LANGUAGE STREQUAL "C" AND RUNTIME_VERSION EQUAL 3.4)
		file(GLOB BACKEND_SRC "${BACKEND_DIR}/*.cpp")
		list(REMOVE_ITEM BACKEND_SRC "${BACKEND_DIR}/drawable.cpp")
		target_sources(${TARGET_NAME} PRIVATE
			${BACKEND_DIR}/Spine34/dxlib_spine_c_34.cpp
			${BACKEND_SRC}
		)

	else()
		file(GLOB BACKEND_SRC "${BACKEND_DIR}/*.cpp")
		target_sources(${TARGET_NAME} PRIVATE ${BACKEND_SRC})
	endif()

	if(SPINE_RUNTIME_DIR)
		file(GLOB SPINE_SRC "${SPINE_RUNTIME_DIR}/src/spine/*.${SPINE_SRC_EXT}")
		target_sources(${TARGET_NAME} PRIVATE ${SPINE_SRC})
	endif()

	target_include_directories(${TARGET_NAME} PRIVATE
		${DXLIB_DIR}
		$<$<BOOL:${SPINE_RUNTIME_DIR}>:${SPINE_RUNTIME_DIR}/include>
		${BACKEND_DIR}
		${RT_DIR}/shared
	)

	target_compile_definitions(${TARGET_NAME} PRIVATE
		SPINE_${VER_NODOT}
		SPINE_RUNTIME_DLL_BUILD
		DX_MAKE
		UNICODE
		_UNICODE
		$<$<STREQUAL:${RUNTIME_LANGUAGE},C>:SPINE_C>
		$<$<NOT:$<STREQUAL:${RUNTIME_LANGUAGE},C>>:SPINE_CPP>
	)

	if(MSVC)
		target_compile_definitions(${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
		target_compile_options(${TARGET_NAME} PRIVATE
			/wd4010
			"$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
		target_link_options(${TARGET_NAME} PRIVATE "/pdbaltpath:%_PDB%")
	endif()

endfunction()

foreach(VER IN LISTS SPINE_VERSIONS_C)
	add_spine_target("C" ${VER})
endforeach()

foreach(VER IN LISTS SPINE_VERSIONS_CPP)
	add_spine_target("Cpp" ${VER})
endforeach()
