import QtQuick 2.9
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3

Window {
    id: mainWindows
    visible: true
    width: 640
    height: 480

    Rectangle {
        id: test
        anchors.fill: parent
        color: "#60C4E4"
        border.color: "#20283F"
        Text {
            id: name
            text: "Test"
            anchors.centerIn: parent
            font.family: "Calibri Light"
            font.pixelSize: 85
            color: "white"
        }
        MouseArea{
            anchors.fill: parent
            onPressed: {
                myserialPortWriter.writeTest();
                test.color = "#20283F";
            }
            onReleased: parent.color = "#60C4E4"
        }
    }
}
