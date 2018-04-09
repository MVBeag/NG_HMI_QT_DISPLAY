import QtQuick 2.0

Item {

    width: 400;
    height: 400;

    property string textEnery: textAreaInput.text
    property int powerAperture: 1

    Rectangle {
        id: background
        anchors.fill: parent
        color: "white"
        objectName: "rect"

        Rectangle {
            id: energyframe
            x: 50; y: 50;
            width: 200; height: 300
            color: "red"
            radius: 4
            border.color: "blue"
        }

        Text {
            id: name
            text: powerAperture
            objectName: "myenergy"
            anchors.centerIn: energyframe
        }

        Rectangle {
            id: incrEnergyFrame
            anchors.left: energyframe.right
            anchors.top: energyframe.top
            width: 50; height: 50
            color: "magenta"
            radius: 4
            border.color: "blue"

            Text {
                text: qsTr("Inc")
                anchors.centerIn: parent
                MouseArea {
                anchors.fill: parent
                onPressed: parent.color = "Red"
                onReleased: powerAperture + 1
                }
            }

        }

        Rectangle {
            id: textArea
            anchors.left: energyframe.right
            anchors.top: incrEnergyFrame.bottom
            width: 50; height: 50
            color: "green"
            radius: 4
            border.color: "blue"

            TextInput {
                id: textAreaInput
                text: "Energy"
                anchors.centerIn: textArea


            }

        }

    }


}
