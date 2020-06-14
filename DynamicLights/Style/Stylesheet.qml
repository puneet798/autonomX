pragma Singleton

import QtQuick 2.12

// NICE TO HAVE: separate singletons for colors/fonts/components/etc
QtObject {
    // COLORS
    property QtObject colors: QtObject {
        readonly property color white: "#ECECEC"
        readonly property color black: "#101010"
        readonly property color darkGrey: "#212121"
    }

    // FONTS
    property QtObject fonts: QtObject {
        readonly property string main: "Overpass"
        readonly property string mainBold: "Overpass Bold"
        readonly property string sub: "Questrial"
    }
}
