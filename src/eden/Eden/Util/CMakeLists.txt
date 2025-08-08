set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

set_source_files_properties(Util.qml
    PROPERTIES
    QT_QML_SINGLETON_TYPE true
)

qt_add_library(EdenUtil STATIC)
qt_add_qml_module(EdenUtil
    URI Eden.Util
    OUTPUT_DIRECTORY EdenUtil
    VERSION 1.0
    QML_FILES

    Util.qml
)

target_link_libraries(EdenUtil
    PRIVATE
    Qt6::Quick
)
