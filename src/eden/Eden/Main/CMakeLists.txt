set(CMAKE_AUTOMOC ON)

qt_add_library(EdenMain STATIC)
qt_add_qml_module(EdenMain
    URI Eden.Main
    VERSION 1.0
    QML_FILES

    Main.qml
    StatusBar.qml
    GameList.qml
    GameGridCard.qml
    MarqueeText.qml
    GameGrid.qml

    GameCarouselCard.qml
    GameCarousel.qml
)
