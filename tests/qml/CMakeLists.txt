find_package(Qt6 ${SPINELOVE_QT_VERSION} EXACT REQUIRED COMPONENTS QuickTest Quick)

add_executable(spinelove_qml_ui_test qml_test_main.cpp)
target_link_libraries(spinelove_qml_ui_test PRIVATE Qt6::QuickTest Qt6::Quick)
target_compile_definitions(spinelove_qml_ui_test PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
spinelove_test_target(spinelove_qml_ui_test)
add_test(NAME viewer.qml_ui COMMAND spinelove_qml_ui_test -input "${CMAKE_CURRENT_SOURCE_DIR}"
    -o "${CMAKE_CURRENT_BINARY_DIR}/qml-ui-results.txt,txt")
set_tests_properties(viewer.qml_ui PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 120)
