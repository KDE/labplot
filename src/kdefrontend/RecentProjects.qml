import QtQuick 2.11
import QtQuick.XmlListModel 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5


ListView {
    id: listView
    spacing: 10
    width: parent.width
    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true
    onWidthChanged: {
        console.log("recent project width changed: " + width)
    }

    model: recentProjects
    delegate: Column {
        id: delegateItem
        width: parent.width
        //width: delegateItem.ListView.view.width
        property string fullUri : modelData
        property int textHeight: 100
        onWidthChanged: {
            console.log("recent project delegate width changed: " + width)
        }

        spacing: 2
        Rectangle {
            width: delegateItem.width
            height: 50 //delegateItem.textHeight

            onWidthChanged: {
                console.log("recent project delegate child width changed: " + width)
            }

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Image {
                    id: helperImage
                    source: helper.getProjectThumbnail(fullUri);
                    fillMode: Image.Stretch
                    sourceSize.width: 48
                    sourceSize.height: 60

                }

                Text{
                    width: parent.width - helperImage.width - parent.spacing
                    Layout.preferredWidth: parent.width - helperImage.width - parent.spacing
                    Layout.minimumWidth: parent.width - helperImage.width - parent.spacing
                    font.pointSize: 14
                    minimumPointSize: 1
                    fontSizeMode: Text.Fit
                    text: delegateItem.fullUri.substring(delegateItem.fullUri.lastIndexOf('/') +1, delegateItem.fullUri.length);
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    //wrapMode: Text.WordWrap
                    //height:paintedHeight

                    onWidthChanged: {
                        console.log("recent project text width changed: " + width)
                    }

                    onPaintedHeightChanged: {
                        if( delegateItem.textHeight <  Math.max(paintedHeight + 2 + 2, 64)) {
                            delegateItem.textHeight = Math.max(paintedHeight + 2 + 2, 64)
                            console.log("Text height for recent: "  + delegateItem.textHeight)
                        }
                    }

                    Component.onCompleted: {
                        if( delegateItem.textHeight <  Math.max(paintedHeight + 2 + 2, 64)) {
                            delegateItem.textHeight = Math.max(paintedHeight + 2 + 2, 64)
                            console.log("Text height for recent: "  + delegateItem.textHeight)
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.color = '#fdffbf' }
                onExited: {parent.color = '#ffffff'}
                onClicked: {mainWindow.recentProjectClicked(delegateItem.fullUri)}
            }
        }

        Rectangle {
            height: 2
            width: delegateItem.width
            color: '#81827c'
        }
    }

    ScrollBar.vertical: ScrollBar { }
}
