set(SPINELOVE_QT_VERSION "6.8.3" CACHE STRING "Exact Qt version used by the QRhi adapter; changing it requires rendering regression checks")
find_package(Qt6 ${SPINELOVE_QT_VERSION} EXACT REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2 ShaderTools Multimedia)
set(CMAKE_AUTOMOC ON)
include("${PROJECT_SOURCE_DIR}/cmake/Live2DWindows.cmake")
file(GLOB SPINELOVE_SDK_HEADERS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/sdk/include/spinelove/*.h")
add_library(spinelove_sdk SHARED ${SPINELOVE_SDK_HEADERS}
    plugin_api/plugin_registry.cpp
    render/scene_snapshot.cpp
    render/texture_loader.cpp
    core/live2d_bridge.cpp
    "${PROJECT_SOURCE_DIR}/main/runtime_shared/spine_runtime_registry.cpp"
    "${PROJECT_SOURCE_DIR}/main/runtime_shared/sl_skeleton_probe.cpp")
target_compile_features(spinelove_sdk PUBLIC cxx_std_17)
target_compile_definitions(spinelove_sdk PRIVATE SPINELOVE_SDK_BUILD NOMINMAX INTERFACE SPINELOVE_SDK_IMPORT)
target_include_directories(spinelove_sdk
    PUBLIC "${PROJECT_SOURCE_DIR}/sdk/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/main" "${PROJECT_SOURCE_DIR}/third_party/stb")
target_link_libraries(spinelove_sdk PUBLIC Qt6::Core Qt6::Gui Qt6::Qml nlohmann_json PRIVATE SpineLove::Runtime spinelove_live2d_windows)
add_library(spinelove_qt_plugin_host STATIC
    plugin_api/plugin_host.h plugin_api/plugin_host.cpp plugin_api/plugin_loader.h plugin_api/plugin_loader.cpp)
target_compile_features(spinelove_qt_plugin_host PUBLIC cxx_std_17)
target_include_directories(spinelove_qt_plugin_host PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(spinelove_qt_plugin_host PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(spinelove_qt_plugin_host PUBLIC spinelove_sdk Qt6::Core Qt6::Widgets)
add_library(spinelove_qt_controller STATIC
    core/viewer_controller.h core/viewer_controller.cpp
    core/viewer_export.cpp
    core/viewer_window.cpp
    core/asset_library.h core/asset_library.cpp
    core/legacy_preferences.h core/legacy_preferences.cpp
    core/export_service.h core/export_service.cpp
    render/slot_outline.h render/slot_outline.cpp render/ui_pointer_observer.h render/ui_pointer_observer.cpp
    render/spine_scene.h render/spine_scene.cpp)
target_link_libraries(spinelove_qt_controller PRIVATE Qt6::GuiPrivate)
target_compile_features(spinelove_qt_controller PUBLIC cxx_std_17)
set_target_properties(spinelove_qt_controller PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(spinelove_qt_controller
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/core" "${PROJECT_SOURCE_DIR}/main" "${PROJECT_SOURCE_DIR}/main/runtime_v2"
    PRIVATE "${PROJECT_SOURCE_DIR}/third_party/stb")
target_link_libraries(spinelove_qt_controller PUBLIC spinelove_sdk spinelove_qt_plugin_host Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick)
if(MSVC)
    target_compile_options(spinelove_qt_controller PRIVATE /utf-8)
    target_compile_options(spinelove_qt_plugin_host PRIVATE /utf-8)
    target_compile_options(spinelove_sdk PRIVATE /utf-8)
endif()
qt_add_executable(spinelove_qt main.cpp)
if(WIN32)
    set_target_properties(spinelove_qt PROPERTIES WIN32_EXECUTABLE TRUE)
    target_sources(spinelove_qt PRIVATE "${PROJECT_SOURCE_DIR}/main/resources/app.rc")
    target_include_directories(spinelove_qt PRIVATE "${PROJECT_SOURCE_DIR}/main")
endif()
target_compile_features(spinelove_qt PRIVATE cxx_std_17)
target_include_directories(spinelove_qt PRIVATE "${PROJECT_SOURCE_DIR}/third_party/stb")
target_link_libraries(spinelove_qt PRIVATE spinelove_qt_controller Qt6::GuiPrivate Qt6::QuickControls2)
if(MSVC)
    target_compile_options(spinelove_qt PRIVATE /utf-8)
endif()
file(GLOB UI_QML CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.qml")
qt_add_resources(spinelove_qt "viewer_ui" PREFIX "/" BASE "${CMAKE_CURRENT_SOURCE_DIR}" FILES Main.qml ${UI_QML})
qt_add_resources(spinelove_qt "viewer_icon" PREFIX "/" BASE "${PROJECT_SOURCE_DIR}" FILES "${PROJECT_SOURCE_DIR}/app.png" "${PROJECT_SOURCE_DIR}/main/resources/app.ico")
file(GLOB UI_LANGUAGES "${CMAKE_CURRENT_SOURCE_DIR}/lang/*.txt")
qt_add_resources(spinelove_qt "viewer_languages" PREFIX "/lang" BASE "${CMAKE_CURRENT_SOURCE_DIR}/lang" FILES ${UI_LANGUAGES})
qt_add_shaders(spinelove_qt "viewer_shaders" PREFIX "/shaders" BASE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" FILES shaders/spine.vert shaders/spine.frag shaders/scene_transition.frag)
add_custom_command(TARGET spinelove_qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/main/resources/NotoSansSC-Regular.ttf" "$<TARGET_FILE_DIR:spinelove_qt>/NotoSansSC-Regular.ttf")
add_custom_command(TARGET spinelove_qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:spinelove_qt>/render_d3d11/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/main/render_d3d11/shaders/sprite.hlsl" "$<TARGET_FILE_DIR:spinelove_qt>/render_d3d11/shaders/sprite.hlsl")

set_target_properties(spinelove_sdk PROPERTIES EXPORT_NAME sdk)
set_target_properties(spinelove_qt_plugin_host PROPERTIES EXPORT_NAME plugin_host)
set_target_properties(spinelove_qt_controller PROPERTIES EXPORT_NAME app_controller)
export(TARGETS spinelove_sdk nlohmann_json spinelove_qt_plugin_host spinelove_qt_controller
    NAMESPACE SpineLove:: FILE "${PROJECT_BINARY_DIR}/sdk/SpineLoveSDKTargets.cmake")
configure_file("${PROJECT_SOURCE_DIR}/cmake/SpineLoveSDKConfig.cmake.in" "${PROJECT_BINARY_DIR}/sdk/SpineLoveSDKConfig.cmake" @ONLY)
