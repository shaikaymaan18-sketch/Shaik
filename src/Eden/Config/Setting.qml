import QtQuick

import Qt.labs.qmlmodels

import Eden.Config

// TODO: make settings independently available (model vs setting?
DelegateChooser {
    id: chooser
    role: "type"

    DelegateChoice {
        roleValue: "bool"

        ConfigCheckbox {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "enumCombo"

        ConfigComboBox {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "intCombo"

        ConfigComboBox {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "intLine"

        ConfigIntLine {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "intSlider"

        ConfigIntSlider {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "time"

        ConfigTimeEdit {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "intSpin"

        ConfigIntSpin {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "stringLine"

        ConfigStringEdit {
            setting: model
            width: ListView.view.width
        }
    }

    DelegateChoice {
        roleValue: "hex"

        ConfigHexEdit {
            setting: model
            width: ListView.view.width
        }
    }
}
