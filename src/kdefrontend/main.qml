import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick 2.4
import QtQuick.Controls 2.3
import QtQuick.Scene2D 2.9
import QtQuick.Controls.Universal 2.0
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.3
import QtWebView 1.1
import QtWebEngine 1.8


Rectangle {
    visible: true
    width: 1920
    height: 1080
    //title: qsTr("Hello World")
    id: mainWindow
    property string initialUrl : "https://labplot.kde.org/2019/04/19/labplot-2-6-released/"
    property alias mainWindow: mainWindow
    signal  recentProjectClicked(url path)

    GridLayout {
        property int spacing: 15
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        id: gridLayout
        anchors.fill: parent
        columnSpacing: 15
        rowSpacing: 15
        layoutDirection: Qt.LeftToRight
        flow: GridLayout.LeftToRight
        columns: 5
        rows: 4


        Frame {
            id: recentProjectsFrame
            //width: mainWindow.width / 5
            //height: mainWindow.height / 3

            Layout.minimumWidth: (parent.width / 5) -  5*gridLayout.spacing
            Layout.minimumHeight: parent.height/4 - 3*gridLayout.spacing
            visible: true
            opacity: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.rowSpan: 1
            Layout.columnSpan: 1
            Layout.column: 0
            Layout.row: 0

            ColumnLayout {
                id: columnLayout
                anchors.fill: parent
                spacing: 20

                Label {
                    id: label
                    color: "#000000"
                    text: qsTr("Recent Projects")
                    styleColor: "#979191"
                    opacity: 1
                    visible: true
                    font.underline: false
                    font.italic: false
                    font.bold: false
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 25
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                }

                ListView {
                    id: listView
                    spacing: 10
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    model: recentProjects
                    delegate: Column {
                        id: delegateItem
                        property string fullUri : modelData
                        width: delegateItem.ListView.view.width
                        spacing: 2
                        Rectangle {
                            width: delegateItem.width
                            height: 25
                            Text{
                                anchors.fill: parent
                                font.pointSize: 14
                                text: delegateItem.fullUri.substring(delegateItem.fullUri.lastIndexOf('/') +1, delegateItem.fullUri.length);
                                font.bold: true
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
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
            }
        }

        Frame {
            id: rectangle2
            Layout.minimumWidth: (3*parent.width / 5) -  5*gridLayout.spacing
            Layout.minimumHeight: parent.height/4 - 3*gridLayout.spacing
            // width: 3 * mainWindow.width / 5
            //height: mainWindow.height / 3
            opacity: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 3
            Layout.rowSpan: 1
            Layout.row: 0
            Layout.column: 1

            Label {
                id: label1
                text: qsTr("Examples")
                styleColor: "#d41919"
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 25
            }

            GridLayout {
                id: gridLayout1
                anchors.top: label1.bottom
                anchors.topMargin: 0
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.left: parent.left
            }
        }

        Frame {
            id: rectangle3
            Layout.minimumWidth: (parent.width / 5) -  5*gridLayout.spacing
            Layout.minimumHeight: parent.height - 3*gridLayout.spacing
            Layout.preferredWidth: parent.width / 5
            Layout.preferredHeight: parent.height
            // width: mainWindow.width / 5
            //height: mainWindow.height
            opacity: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.rowSpan: 4
            Layout.columnSpan: 1
            Layout.row: 0
            Layout.column: 4

            ColumnLayout {
                anchors.fill: parent

                Label {
                    id: label2
                    text: qsTr("News")
                    Layout.fillWidth: true
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 0
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 25
                }

                RssNews{id:newsFeed}
            }

        }

        Frame {
            id: helpFrame
            //width: mainWindow.width / 5
            // height: mainWindow.height / 3
            Layout.minimumWidth: (parent.width / 5) -  5*gridLayout.spacing
            Layout.minimumHeight: parent.height/4 - 3*gridLayout.spacing
            opacity: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.rowSpan: 1
            Layout.columnSpan: 1
            Layout.row: 1
            Layout.column: 0

            ColumnLayout {
                id: columnLayout2
                anchors.fill: parent
                spacing: 30

                Label {
                    id: label3
                    text: qsTr("Help")
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 25
                    Layout.fillWidth: true
                }

                ListView {
                    id: listView2
                    x: 0
                    y: 0
                    width: 110
                    height: 160
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    ScrollBar.vertical: ScrollBar { }
                    model: ListModel {
                        ListElement {
                            name: "Documentation"
                            link: "https://docs.kde.org/trunk5/en/extragear-edu/labplot2/index.html"
                        }

                        ListElement {
                            name: "FAQ"
                            link: "https://docs.kde.org/trunk5/en/extragear-edu/labplot2/faq.html"
                        }

                        ListElement {
                            name: "Features"
                            link: "https://labplot.kde.org/features/"
                        }

                        ListElement {
                            name: "Support"
                            link: "https://labplot.kde.org/support/"
                        }
                    }
                    delegate: Rectangle {
                        width: parent.width
                        height: 25
                        RowLayout {
                            id: row3
                            width: parent.width
                            Layout.fillHeight: true

                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 5
                                height: 5
                                color: "#7a7d82"
                            }

                            Label {
                                text: name
                                font.bold: true
                                font.pixelSize: 18
                            }
                            spacing: 10
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: {parent.color = '#fdffbf' }
                            onExited: {parent.color = '#ffffff'}
                            onClicked: {Qt.openUrlExternally(link)}
                        }
                    }
                }
            }
        }

        Frame {
            id: rectangle5
            // width: 3 * mainWindow.width / 5
            // height: mainWindow.height / 3
            Layout.minimumWidth: (3*parent.width / 5) -  5*gridLayout.spacing
            Layout.minimumHeight: parent.height/4 - 3*gridLayout.spacing
            opacity: 1
            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.rowSpan: 1
            Layout.row: 1
            Layout.column: 1

            ColumnLayout {
                id: columnLayout3
                anchors.fill: parent

                Label {
                    id: label4
                    text: qsTr("Start exploring data")
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 25
                    Layout.fillWidth: true
                }

                RowLayout {
                    id: rowLayout
                    width: 100
                    height: 100
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    ColumnLayout {
                        id: columnLayout4
                        width: 100
                        height: 100
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        Label {
                            id: label5
                            text: qsTr("Medicine")
                            font.pointSize: 20
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                        }

                        ListView {
                            id: listView3
                            width: 110
                            height: 160
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            model: ListModel {
                                ListElement {
                                    name: "Grey"
                                    colorCode: "grey"
                                }

                                ListElement {
                                    name: "Red"
                                    colorCode: "red"
                                }

                                ListElement {
                                    name: "Blue"
                                    colorCode: "blue"
                                }

                                ListElement {
                                    name: "Green"
                                    colorCode: "green"
                                }
                            }
                            delegate: Item {
                                x: 5
                                width: 80
                                height: 40
                                Row {
                                    id: row4
                                    Rectangle {
                                        width: 40
                                        height: 40
                                        color: colorCode
                                    }

                                    Text {
                                        text: name
                                        anchors.verticalCenter: parent.verticalCenter
                                        font.bold: true
                                    }
                                    spacing: 10
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        id: columnLayout5
                        width: 100
                        height: 100
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        Label {
                            id: label6
                            text: qsTr("Nature")
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pointSize: 20
                        }

                        ListView {
                            id: listView4
                            x: 0
                            y: 0
                            width: 110
                            height: 160
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            model: ListModel {
                                ListElement {
                                    name: "Grey"
                                    colorCode: "grey"
                                }

                                ListElement {
                                    name: "Red"
                                    colorCode: "red"
                                }

                                ListElement {
                                    name: "Blue"
                                    colorCode: "blue"
                                }

                                ListElement {
                                    name: "Green"
                                    colorCode: "green"
                                }
                            }
                            delegate: Item {
                                x: 5
                                width: 80
                                height: 40
                                Row {
                                    id: row5
                                    Rectangle {
                                        width: 40
                                        height: 40
                                        color: colorCode
                                    }

                                    Text {
                                        text: name
                                        anchors.verticalCenter: parent.verticalCenter
                                        font.bold: true
                                    }
                                    spacing: 10
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        id: columnLayout6
                        width: 100
                        height: 100
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        Label {
                            id: label7
                            text: qsTr("Physics")
                            horizontalAlignment: Text.AlignHCenter
                            font.pointSize: 20
                            Layout.fillWidth: true
                        }

                        ListView {
                            id: listView5
                            x: 0
                            y: 0
                            width: 110
                            height: 160
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            model: ListModel {
                                ListElement {
                                    name: "Grey"
                                    colorCode: "grey"
                                }

                                ListElement {
                                    name: "Red"
                                    colorCode: "red"
                                }

                                ListElement {
                                    name: "Blue"
                                    colorCode: "blue"
                                }

                                ListElement {
                                    name: "Green"
                                    colorCode: "green"
                                }
                            }
                            delegate: Item {
                                x: 5
                                width: 80
                                height: 40
                                Row {
                                    id: row6
                                    Rectangle {
                                        width: 40
                                        height: 40
                                        color: colorCode
                                    }

                                    Text {
                                        text: name
                                        anchors.verticalCenter: parent.verticalCenter
                                        font.bold: true
                                    }
                                    spacing: 10
                                }
                            }
                        }
                    }
                }
            }
        }

        Frame {
            id: rectangle
            // width: 4 * mainWindow.width / 5
            // height: mainWindow.height / 3
            Layout.minimumWidth: (4* parent.width / 5) -  5*gridLayout.spacing
            Layout.minimumHeight: 2 * parent.height/4 - 3*gridLayout.spacing
            opacity: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 4
            Layout.rowSpan: 2
            Layout.column: 0
            Layout.row: 2

            ColumnLayout {
                id: columnLayout7
                anchors.fill: parent

                Label {
                    id: label8
                    text: qsTr("What's new in this release")
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 30
                    Layout.fillWidth: true
                }

                WebView {
                    id: webView
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    url: initialUrl
                }
            }
        }
    }
}
