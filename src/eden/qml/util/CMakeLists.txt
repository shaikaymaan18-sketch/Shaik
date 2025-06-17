set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

set_source_files_properties(Util.qml
    PROPERTIES
    QT_QML_SINGLETON_TYPE true
)

qt_add_library(edenUtil STATIC)
qt_add_qml_module(edenUtil
    URI org.eden_emu.util
    OUTPUT_DIRECTORY EdenUtil
    VERSION 1.0
    QML_FILES

    Util.qml
)

target_link_libraries(edenUtil
    PRIVATE
    Qt6::Quick
)
