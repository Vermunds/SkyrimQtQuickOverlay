import QtQuick
import QtQuick.Controls
import QtQuick.Timeline 1.0

Rectangle {
    id: root
    color: "transparent"

    Label {
        id: label
        x: 0
        y: 0
        color: "white"
        text: "Qt Quick overlay in Skyrim"
        font.pointSize: 18
    }

    Timeline {
        id: timeline
        animations: [
            TimelineAnimation {
                id: timelineAnimation
                running: true
                loops: Animation.Infinite
                duration: 3000
                to: 3000
                from: 0
                pingPong: true
            }
        ]
        endFrame: 3000
        enabled: true
        startFrame: 0


        KeyframeGroup {
            target: label
            property: "x"
            Keyframe {
                value: (root.width - label.width) / 2
                frame: 1500
            }

            Keyframe {
                value: (root.width - label.width)
                frame: 3000
            }
        }

        KeyframeGroup {
            target: label
            property: "y"
            Keyframe {
                value: (root.height - label.height) / 2
                frame: 1500
            }

            Keyframe {
                value: (root.height - label.height)
                frame: 3000
            }
        }
    }

    Component.onCompleted: {console.log("QML loaded.")}
}