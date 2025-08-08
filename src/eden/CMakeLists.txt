set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.7)

add_subdirectory(Eden)
