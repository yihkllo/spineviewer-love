find_package(Qt6 ${SPINELOVE_QT_VERSION} EXACT REQUIRED COMPONENTS Gui Quick QuickControls2 Test)

function(spinelove_app_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE spinelove_qt_controller ${ARGN})
    spinelove_test_target(${name})
endfunction()

spinelove_app_test(spinelove_qt_controller_test viewer_controller_test.cpp Qt6::Test)
target_include_directories(spinelove_qt_controller_test PRIVATE "${PROJECT_SOURCE_DIR}/tests/runtime")
add_test(NAME viewer.controller_interactions COMMAND spinelove_qt_controller_test -platform offscreen)

spinelove_app_test(spinelove_scene_viewport_test scene_viewport_lifecycle_test.cpp Qt6::Test Qt6::GuiPrivate Qt6::QuickControls2)
add_test(NAME viewer.scene_viewport_lifecycle COMMAND spinelove_scene_viewport_test -o "${CMAKE_CURRENT_BINARY_DIR}/viewport-lifecycle-results.txt,txt")
set_tests_properties(viewer.scene_viewport_lifecycle PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=2" TIMEOUT 30)

spinelove_app_test(spinelove_qt_asset_library_test asset_library_test.cpp)
add_test(NAME viewer.asset_library COMMAND spinelove_qt_asset_library_test)

spinelove_app_test(spinelove_qt_export_service_test export_service_test.cpp)
add_test(NAME viewer.export_service COMMAND spinelove_qt_export_service_test)

spinelove_app_test(spinelove_qt_slot_outline_test slot_outline_test.cpp)
add_test(NAME viewer.slot_outline COMMAND spinelove_qt_slot_outline_test)

add_executable(spinelove_qt_texture_loader_test texture_loader_test.cpp)
target_link_libraries(spinelove_qt_texture_loader_test PRIVATE spinelove_sdk)
spinelove_test_target(spinelove_qt_texture_loader_test)
add_test(NAME viewer.texture_loader COMMAND spinelove_qt_texture_loader_test)

add_executable(spinelove_qt_interaction_rules_test interaction_rules_test.cpp)
target_link_libraries(spinelove_qt_interaction_rules_test PRIVATE spinelove_sdk)
set_target_properties(spinelove_qt_interaction_rules_test PROPERTIES AUTOMOC OFF)
spinelove_test_target(spinelove_qt_interaction_rules_test)
add_test(NAME viewer.interaction_rules COMMAND spinelove_qt_interaction_rules_test)

if(WIN32)
    spinelove_app_test(spinelove_asset_library_audit asset_library_audit.cpp Shlwapi)
    target_sources(spinelove_asset_library_audit PRIVATE sl_path_util.cpp sl_path_util.h)
endif()
