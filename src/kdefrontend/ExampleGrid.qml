import QtQuick 2.6
import QtQuick.XmlListModel 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

GridView {
    id: exampleGrid
    Layout.fillHeight: true
    Layout.fillWidth: true
    cellWidth: width/4
    cellHeight: Math.min(height*0.9, 160)
    ScrollBar.vertical: ScrollBar{}
    clip: true

    model: helper.getExampleProjects();
    delegate: Rectangle {
        id: exampleDelegate
        property string name : modelData
        width: exampleGrid.cellWidth - 5
        height: exampleGrid.cellHeight - 5

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            //onEntered: {exampleDelegate.color = '#fdffbf'}
            //onExited: {exampleDelegate.color = '#ffffff'}
            onClicked: {mainWindow.openExampleProject(exampleDelegate.name)}
        }

        ColumnLayout {
            anchors.fill: parent
            //Layout.fillHeight: true
            //Layout.fillWidth: true
            spacing: 5
            Image {
                id: exampleImage
                source: helper.getExampleProjectThumbnail(name)
                fillMode: Image.Stretch
                sourceSize.width: Math.min(120, exampleDelegate.width)
                sourceSize.height: Math.min(100, exampleDelegate.height * 0.6)
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                Layout.preferredWidth: parent.width
                Layout.minimumWidth: parent.width
                width: parent.width
                Layout.preferredHeight:  exampleDelegate.height  * 0.15
                Layout.minimumHeight:  exampleDelegate.height  * 0.15
                height: exampleDelegate.height * 0.15
                wrapMode: Text.WordWrap
                text: exampleDelegate.name
                font.pixelSize: 14
                minimumPixelSize: 10
                fontSizeMode: Text.Fit
                font.bold: true
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                Layout.preferredWidth: parent.width
                Layout.minimumWidth: parent.width
                width: parent.width
                Layout.preferredHeight:  exampleDelegate.height  * 0.25
                Layout.minimumHeight:  exampleDelegate.height  * 0.25
                height: exampleDelegate.height * 0.25
                wrapMode: Text.WordWrap
                text: helper.getExampleProjectTags(exampleDelegate.name)
                minimumPixelSize: 6
                font.pixelSize: 12
                fontSizeMode: Text.Fit
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
