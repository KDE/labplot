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
    property string currentCategory: ''
    property string currentSubcategory: ''
    property string initialUrl : "https://labplot.kde.org/2019/04/19/labplot-2-6-released/"
    property alias mainWindow: mainWindow
    signal  recentProjectClicked(url path)
    signal datasetClicked(string category, string subcategory, string dataset)

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
            id: datasetFrame

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
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    spacing: 10

                    ListView {
                        id: categoryList
                        Layout.minimumWidth: 150
                        spacing: 10
                        Layout.fillHeight: true

                        ScrollBar.vertical: ScrollBar { }
                        Component.onCompleted: console.log("Model: " +  datasetModel.categories())
                        model: datasetModel.categories()
                        delegate:Rectangle {
                            width: parent.width
                            height: 25
                            id: categoryDelegate
                            property string categoryName : modelData
                            property bool selected: ListView.isCurrentItem

                            RowLayout {
                                spacing: 5
                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 5
                                    height: 5
                                    color: "#7a7d82"
                                }

                                Label {
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    text: categoryDelegate.categoryName
                                    font.bold: true
                                    font.pixelSize: 18
                                    color: selected ? "#d69f00" : "#000000"
                                    scale: selected ? 1.15 : 1.0
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                    Behavior on scale { PropertyAnimation { duration: 300 } }

                                    Component.onCompleted: console.log("Category name: " +  categoryDelegate.categoryName)
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log("Category name: " +  categoryDelegate.categoryName + "Clicked")
                                    categoryDelegate.ListView.view.currentIndex = index
                                    if (mainWindow.currentCategory != categoryName)
                                        mainWindow.currentCategory = categoryName
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        width: 5
                        color: "grey"
                    }

                    ListView {
                        id: subcategoryList
                        spacing: 20
                        Layout.fillHeight: true
                        //Layout.fillWidth: true
                        width: 250
                        //ScrollBar.horizontal: ScrollBar {}
                        ScrollBar.vertical: ScrollBar{}
                        //orientation: ListView.Horizontal
                        //clip: true
                        model: datasetModel.subcategories(mainWindow.currentCategory)
                        delegate: Rectangle {
                            width: parent.width
                            height: 25
                            id: subcategoryDelegate
                            property string subcategoryName : modelData
                            property bool selected: ListView.isCurrentItem

                            RowLayout {
                                spacing: 10
                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 5
                                    height: 5
                                    color: "#7a7d82"
                                }

                                Label {
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    text: subcategoryDelegate.subcategoryName
                                    font.bold: true
                                    font.pixelSize: 18
                                    color: selected ? "#d69f00" : "#000000"
                                    scale: selected ? 1.15 : 1.0
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                    Behavior on scale { PropertyAnimation { duration: 300 } }

                                    Component.onCompleted: {
                                        console.log("Subcategory name: " +  subcategoryDelegate.subcategoryName)
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log("Subcategory name: " +  subcategoryDelegate.subcategoryName + "Clicked")
                                    subcategoryDelegate.ListView.view.currentIndex = index
                                    if (mainWindow.currentSubcategory != subcategoryName)
                                        mainWindow.currentSubcategory = subcategoryName
                                }
                            }
                        }


                        /*Rectangle {
                            id: subcategoryDelegate
                            width: 150
                            Layout.fillHeight: true
                            property string subcategoryName: modelData
                            property variant datasets : datasetModel.datasets(mainWindow.currentCategory, subcategoryName)



                                ColumnLayout {
                                    spacing: 10
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 20

                                        Text {
                                            anchors.fill: parent
                                            text: subcategoryDelegate.subcategoryName
                                            font.bold: true
                                            font.underline: false
                                            font.pixelSize: 18

                                            Component.onCompleted: console.log("Subcategory name: " +  subcategoryDelegate.subcategoryName)
                                        }
                                    }

                                    Repeater {
                                        model: subcategoryDelegate.datasets
                                        id: subItemRepeater
                                        delegate: Rectangle {
                                            height: 20
                                            width: 150
                                            border.color: "black"
                                            border.width: 2

                                            Text {
                                                anchors.verticalCenter: parent.verticalCenter
                                                font.pixelSize: 16
                                                text: modelData
                                            }
                                        }
                                    }*/



                        /*ListView {
                                    id: datasetList
                                    spacing: 10
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    orientation: ListView.Vertical
                                    ScrollBar.vertical: ScrollBar { }
                                    model:
                                    delegate: Rectangle {
                                        id: datasetDelegate
                                        //Layout.fillWidth: true
                                        height: 20
                                        property string datasetName: modelData

                                        Text {
                                            anchors.fill: parent
                                            text: datasetDelegate.datasetName
                                            font.pixelSize: 16

                                            Component.onCompleted: console.log("Dataset name: " +  datasetDelegate.datasetName + "  " + datasetList.count)
                                        }
                                    }
                                }*/
                        /*}

                }*/
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        width: 5
                        color: "grey"
                    }

                    GridView {
                        id: datasetGrid
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        cellWidth: width/4
                        cellHeight: 40

                        model: datasetModel.datasets(mainWindow.currentCategory, mainWindow.currentSubcategory)

                        delegate: Rectangle {
                            id: datasetDelegate
                            property string datasetName : modelData
                            property bool selected: (index == GridView.currentIndex)
                            //Layout.fillHeight: true
                            //Layout.fillWidth: true
                            //width: datasetText.paintedWidth + datasetBullet.width
                            width: textWidth
                            height: textHeight
                            //border.color: 'black'
                            //border.width: 2

                            property int textWidth: 200
                            property int textHeight: 40

                            RowLayout {
                                id: datasetRow
                                spacing: 5
                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle {
                                    id: datasetBullet
                                    width: 5
                                    height: 5
                                    color: "#7a7d82"
                                }

                                Label {
                                    id: datasetText
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    text: datasetDelegate.datasetName
                                    font.bold: true
                                    font.pixelSize: 18
                                    color: selected ? "#d69f00" : "#000000"
                                    scale: selected ? 1.15 : 1.0
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                    Behavior on scale { PropertyAnimation { duration: 300 } }

                                    Component.onCompleted: {
                                        console.log("Dataset name: " +  datasetDelegate.datasetName)
                                        datasetDelegate.textHeight = paintedHeight
                                        datasetDelegate.textWidth = paintedWidth + datasetBullet.width + datasetRow.spacing
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log("Dataset name: " +  datasetDelegate.datasetName + "Clicked")
                                    mainWindow.datasetClicked(mainWindow.currentCategory, mainWindow.currentSubcategory, datasetDelegate.datasetName)
                                    /*subcategoryDelegate.ListView.view.currentIndex = index
                                    if (mainWindow.currentSubcategory != subcategoryName)
                                        mainWindow.currentSubcategory = subcategoryName*/
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
