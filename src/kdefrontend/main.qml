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

//import labplot.datasetmodel 1.0


Rectangle {
    visible: true
    width: 1920
    height: 1080
    //title: qsTr("Hello World")
    id: mainWindow
    property string currentCategory: ''
    property string currentSubcategory: ''
    property string currentDataset: ''
    property string initialUrl : "https://labplot.kde.org/2019/04/19/labplot-2-6-released/"
    property alias mainWindow: mainWindow
    signal  recentProjectClicked(url path)
    signal datasetClicked(string category, string subcategory, string dataset)
    signal openDataset()



    Connections {
        target: helper
        onDatasetFound:{
            datasetTitle.text = helper.datasetName()
            console.log("Title width: " + datasetTitle.width)
            datasetDescription.text = helper.datasetDescription()
            console.log("Description width: " + datasetDescription.width)
            console.log("Description height: " + datasetDescription.paintedHeight  + "  " + height)
            datasetRows.text = helper.datasetRows()
            datasetColumns.text = helper.datasetColumns()
        }
        onDatasetNotFound:{
            datasetTitle.text = "-"
            datasetDescription.text = "-"
            datasetRows.text = "-"
            datasetColumns.text = "-";
        }
        onShowFirstDataset:{
            datasetClicked(mainWindow.currentCategory, mainWindow.currentSubcategory, mainWindow.currentDataset)
        }
    }


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
                    Layout.fillWidth: true
                    Layout.minimumHeight: paintedHeight
                }

                ListView {
                    id: listView
                    spacing: 10
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    model: recentProjects
                    delegate: Column {
                        id: delegateItem
                        property string fullUri : modelData
                        width: delegateItem.ListView.view.width
                        spacing: 2
                        Rectangle {
                            width: delegateItem.width
                            height: 50

                            RowLayout {
                                anchors.fill: parent
                                spacing: 10
                                height: parent.height

                                Image {
                                    source: helper.getProjectThumbnail(fullUri);
                                    fillMode: Image.Stretch
                                    sourceSize.width: 48
                                    sourceSize.height: 60
                                }

                                Text{
                                    Layout.fillWidth: true
                                    font.pointSize: 14
                                    text: delegateItem.fullUri.substring(delegateItem.fullUri.lastIndexOf('/') +1, delegateItem.fullUri.length);
                                    font.bold: true
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
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
                        width: textWidth
                        implicitWidth: textWidth
                        spacing: 10
                        Layout.fillHeight: true
                        property int textWidth: 100


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
                                id: categoryRow
                                spacing: 10
                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle {
                                    id: categoryBullet
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 5
                                    height: 5
                                    color: "#7a7d82"
                                }

                                Label {
                                    id: categoryLabel
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    text: categoryDelegate.categoryName
                                    font.bold: true
                                    font.pixelSize: 18
                                    color: selected ? "#d69f00" : "#000000"
                                    scale: selected ? 1.15 : 1.0
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                    Behavior on scale { PropertyAnimation { duration: 300 } }

                                    Component.onCompleted:  {
                                        console.log("Category name: " +  categoryDelegate.categoryName)
                                        if(index == 0) {
                                            mainWindow.currentCategory =  categoryDelegate.categoryName
                                            categoryDelegate.ListView.view.currentIndex = index
                                            console.log("Set current categ: " +   mainWindow.currentCategory)
                                        }

                                        console.log("Category size: " + (paintedWidth + categoryBullet.width + categoryRow.spacing))
                                        if(categoryList.textWidth < paintedWidth + categoryBullet.width + categoryRow.spacing)
                                            categoryList.textWidth = paintedWidth + categoryBullet.width + categoryRow.spacing
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log("Category name: " +  categoryDelegate.categoryName + "Clicked")
                                    categoryDelegate.ListView.view.currentIndex = index
                                    if (mainWindow.currentCategory != categoryName)
                                        mainWindow.currentCategory = categoryName

                                    console.log("Category size: " + (categoryLabel.paintedWidth + categoryBullet.width + categoryRow.spacing))
                                    if(categoryList.textWidth < categoryLabel.paintedWidth + categoryBullet.width + categoryRow.spacing)
                                        categoryList.textWidth = categoryLabel.paintedWidth + categoryBullet.width + categoryRow.spacing
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
                        width: textWidth
                        implicitWidth: textWidth
                        //ScrollBar.horizontal: ScrollBar {}
                        ScrollBar.vertical: ScrollBar{}
                        //orientation: ListView.Horizontal
                        //clip: true
                        property int textWidth: 100

                        model: datasetModel.subcategories(mainWindow.currentCategory)
                        delegate: Rectangle {
                            width: parent.width
                            height: 25
                            id: subcategoryDelegate
                            property string subcategoryName : modelData
                            property bool selected: ListView.isCurrentItem

                            RowLayout {
                                id: subcategoryRow
                                spacing: 10
                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle {
                                    id: subcategoryBullet
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

                                        if(index == 0) {
                                            mainWindow.currentSubcategory = subcategoryDelegate.subcategoryName
                                            subcategoryDelegate.ListView.view.currentIndex = index
                                        }

                                        console.log("Subcategory size: " + (paintedWidth + subcategoryBullet.width + subcategoryRow.spacing))

                                        if(subcategoryList.textWidth < paintedWidth + subcategoryBullet.width + subcategoryRow.spacing) {
                                            subcategoryList.textWidth = paintedWidth + subcategoryBullet.width + subcategoryRow.spacing
                                            console.log("Subcategory size bigger, new value: " + subcategoryList.textWidth + " " + subcategoryList.width)
                                        }
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
                            property bool selected: (index == datasetGrid.currentIndex)
                            width: textWidth
                            height: textHeight

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

                                        if(index == 0) {
                                            console.log("index 0  " +  mainWindow.currentCategory +"   subcat:  " + mainWindow.currentSubcategory + " dtaset  " +datasetDelegate.datasetName);
                                            datasetGrid.currentIndex = index
                                            mainWindow.currentDataset = datasetDelegate.datasetName
                                            mainWindow.datasetClicked(mainWindow.currentCategory, mainWindow.currentSubcategory, datasetDelegate.datasetName)
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    datasetGrid.currentIndex = index
                                    console.log("Dataset name: " +  datasetDelegate.datasetName + "Clicked")
                                    mainWindow.currentDataset = datasetDelegate.datasetName
                                    mainWindow.datasetClicked(mainWindow.currentCategory, mainWindow.currentSubcategory, datasetDelegate.datasetName)
                                }
                            }

                            Component.onCompleted: {
                                console.log("current index: " +  datasetGrid.currentIndex + " index " + index)
                            }
                        }

                        Component.onCompleted: {
                            console.log("GridView completed")
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        width: 5
                        color: "grey"
                    }


                    ScrollView {
                        Layout.minimumWidth: datasetFrame.width / 5
                        Layout.preferredWidth: datasetFrame.width / 5
                        width: datasetFrame.width / 5
                        Layout.fillHeight: true
                        contentHeight: datasetDescriptionColumn.height
                        clip: true
                        ColumnLayout {
                            id: datasetDescriptionColumn
                            //anchors.fill: parent
                            Layout.minimumWidth: datasetFrame.width / 5
                            Layout.preferredWidth: datasetFrame.width / 5
                            width: datasetFrame.width / 5
                            //Layout.fillWidth: true
                            //Layout.fillHeight: true
                            spacing: 10
                            //ScrollBar.vertical: ScrollBar{}

                            /*ScrollBar {
                                id: vbar
                                hoverEnabled: true
                                active: hovered || pressed
                                orientation: Qt.Vertical
                                //size: frame.height / content.height
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                            }*/

                            Row {
                                width: datasetDescriptionColumn.width
                                spacing: 5
                                Text {
                                    id: datasetTitleLabel
                                    text: "Full name: "
                                    font.pixelSize: 14
                                    font.bold: true
                                }

                                Text {
                                    id: datasetTitle
                                    //Layout.fillWidth: true
                                    width: datasetDescriptionColumn.width - datasetTitleLabel.paintedWidth
                                    text: "-"
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 14
                                }
                            }

                            Row {
                                width: parent.width
                                height: Math.max(datasetDescription.textHeight, datasetDescriptionLabel.paintedHeight)
                                spacing: 5
                                Text {
                                    id: datasetDescriptionLabel
                                    text: "Description: "
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                Text {
                                    id: datasetDescription
                                    property double textHeight: paintedHeight

                                    text: ""
                                    width: datasetDescriptionColumn.width - datasetDescriptionLabel.paintedWidth
                                    Layout.preferredWidth: datasetDescriptionColumn.width - datasetDescriptionLabel.paintedWidth
                                    Layout.minimumWidth: datasetDescriptionColumn.width - datasetDescriptionLabel.paintedWidth
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 12

                                    Component.onCompleted:{
                                        console.log("First Description width: " + width +  " " + (datasetDescriptionColumn.width - datasetDescriptionLabel.paintedWidth))
                                        console.log("First Description height: " + paintedHeight +  " " + parent.height)
                                    }
                                }
                            }

                            Row {
                                width: parent.width
                                spacing: 5
                                Text {
                                    id: datasetColumnsLabel
                                    text: "Columns: "
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                Text {
                                    id: datasetColumns
                                    text: "-"
                                    width: datasetDescriptionColumn.width - datasetColumnsLabel.paintedWidth
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 14
                                }
                            }

                            Row {
                                width: parent.width
                                spacing: 5
                                Text {
                                    id: datasetRowsLabel
                                    text: "Rows: "
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                Text {
                                    id: datasetRows
                                    text: "-"
                                    width: datasetDescriptionColumn.width - datasetRowsLabel.paintedWidth
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 14
                                }
                            }

                            Rectangle {
                                width: datasetButtonText.paintedWidth + 10
                                height: datasetButtonText.paintedHeight + 10
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                                //anchors.horizontalCenter: parent.horizontalCenter
                                //border.color: 'black'
                                //border.width: 3
                                color:'#dfe3ee'

                                Text {
                                    id: datasetButtonText
                                    text: "Open dataset"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 14
                                    font.bold: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        mainWindow.openDataset()
                                    }
                                }
                            }

                            Component.onCompleted: console.log("Width of last column: " + datasetFrame.width / 5 + "  " + width)
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
