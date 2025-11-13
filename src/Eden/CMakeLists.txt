set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_standard_project_setup(REQUIRES 6.4)

include(EdenModule)

add_subdirectory(Interface)
add_subdirectory(Models)

add_subdirectory(Config)
add_subdirectory(Constants)
add_subdirectory(Items)
add_subdirectory(Util)
add_subdirectory(Main)

add_subdirectory(Native)
