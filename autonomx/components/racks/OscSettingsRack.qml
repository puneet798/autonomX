import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import "qrc:/stylesheet"
import "../ui"
import "../fields"

ColumnLayout {
    spacing: Stylesheet.field.spacing

    RowLayout {
        spacing: Stylesheet.field.spacing
        Layout.fillWidth: true

        CheckField {
            id: enableOscIn
            controlColor: Stylesheet.colors.inputs[0]
            propName: ""
        }

        TextField {
            labelText: "Address"
            activated: enableOscIn.on
            propName: "oscInputAddress"
        }

        NumberField {
            labelText: "Port"
            activated: enableOscIn.on
            propName: "oscInputPort"
        }
    }

    RowLayout {
        spacing: Stylesheet.field.spacing
        Layout.fillWidth: true

        CheckField {
            id: enableOscOut
            controlColor: Stylesheet.colors.outputs[0]
            propName: ""
        }

        TextField {
            labelText: "Address"
            activated: enableOscOut.on
            propName: "oscOutputAddressTarget"
        }
        TextField {
            labelText: "Host"
            activated: enableOscOut.on
            propName: "oscOutputAddressHost"
            // TODO: input masking to only allow a standard IP address format
        }
        NumberField {
            labelText: "Port"
            activated: enableOscOut.on
            propName: "oscOutputPort"
        }
    }
}
