import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: scroll

    WheelHandler {
        target: scroll
        onWheel: event => {
                     const sensitivity = 1 / 1500
                     scroll.ScrollBar.vertical.position -= event.angleDelta.y * sensitivity
                     scroll.ScrollBar.vertical.position = Math.max(
                         Math.min(scroll.ScrollBar.vertical.position,
                                  1.0 - scroll.ScrollBar.vertical.size), 0.0)
                 }
    }
}
