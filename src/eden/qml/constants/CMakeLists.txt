set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

set_source_files_properties(Constants.qml
    PROPERTIES
    QT_QML_SINGLETON_TYPE true
)

qt_add_library(edenConstants STATIC)
qt_add_qml_module(edenConstants
    URI org.eden_emu.constants
    OUTPUT_DIRECTORY EdenConstants
    VERSION 1.0
    QML_FILES

    Constants.qml
)

target_link_libraries(edenConstants
    PRIVATE
    Qt6::Quick
)
