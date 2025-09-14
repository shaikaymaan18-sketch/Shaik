set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.7)

include(EdenModule)

include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(Interface)
add_subdirectory(Models)

add_subdirectory(Config)
add_subdirectory(Constants)
add_subdirectory(Items)
add_subdirectory(Util)
add_subdirectory(Main)

add_subdirectory(Native)
