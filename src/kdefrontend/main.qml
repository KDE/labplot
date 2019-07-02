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
    id: mainWindow
    width: 1920
    height: 1080
    property int spacing: 10
    visible: true
    property string currentCategory: ''
    property string currentSubcategory: ''
    property string currentDataset: ''
    property string initialUrl : "https://labplot.kde.org/2019/04/19/labplot-2-6-released/"
    property alias mainWindow: mainWindow
    signal  recentProjectClicked(url path)
    signal datasetClicked(string category, string subcategory, string dataset)
    signal openDataset()
    signal openExampleProject(string name)



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

    Frame {
        id: recentProjectsFrame
        width: mainWindow.width / 5 - 4*mainWindow.spacing
        height: mainWindow.height / 4 - 4*mainWindow.spacing
        anchors.top: parent.top
        anchors.topMargin: mainWindow.spacing
        anchors.left: parent.left
        anchors.leftMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.left: parent.right
            anchors.rightMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis;}
                onMouseXChanged: {
                    if(drag.active){
                        recentProjectsFrame.width = recentProjectsFrame.width + mouseX
                        exampleProjects.width = exampleProjects.width - mouseX
                        if(recentProjectsFrame.width < 150){
                            recentProjectsFrame.width = 150
                            exampleProjects.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }
                        if(exampleProjects.width < 150) {
                            exampleProjects.width = 150
                            recentProjectsFrame.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }

                    }
                }
            }
        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.top: parent.bottom
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        recentProjectsFrame.height = recentProjectsFrame.height + mouseY
                        exampleProjects.height = exampleProjects.height + mouseY
                        helpFrame.height = helpFrame.height - mouseY
                        datasetFrame.height = datasetFrame.height - mouseY
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.height = 100
                            exampleProjects.height = 100
                            helpFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            recentProjectsFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            exampleProjects.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }

        }

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
                wrapMode: Text.WordWrap
            }

            ListView {
                id: listView
                spacing: 10
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                onWidthChanged: {
                    console.log("recent project width changed: " + width)
                }

                model: recentProjects
                delegate: Column {
                    id: delegateItem
                    property string fullUri : modelData
                    width: delegateItem.ListView.view.width
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
                                source: helper.getProjectThumbnail(fullUri);
                                fillMode: Image.Stretch
                                sourceSize.width: 48
                                sourceSize.height: 60

                            }

                            Text{
                                font.pointSize: 14
                                text: delegateItem.fullUri.substring(delegateItem.fullUri.lastIndexOf('/') +1, delegateItem.fullUri.length);
                                font.bold: true
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
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
        }
    }

    Frame {
        id: exampleProjects
        width: 3 * mainWindow.width / 5 - 4*mainWindow.spacing
        height: mainWindow.height / 4 - 4*mainWindow.spacing
        anchors.top: parent.top
        anchors.topMargin: mainWindow.spacing
        anchors.left: recentProjectsFrame.right
        anchors.leftMargin: mainWindow.spacing
        anchors.right: newsSection.left
        anchors.rightMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.left: parent.right
            anchors.rightMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        newsSection.width = newsSection.width - mouseX
                        exampleProjects.width = exampleProjects.width + mouseX
                        datasetFrame.width = datasetFrame.width + mouseX
                        releaseSection.width = releaseSection.width + mouseX
                        if(exampleProjects.width < 150
                                || datasetFrame.width < 150){


                            newsSection.width = mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 150 - 4*mainWindow.spacing
                            exampleProjects.width = mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing
                            releaseSection.width = Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 150

                        }
                        if(newsSection.width < 150) {
                            newsSection.width = 150
                            exampleProjects.width = mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing
                            releaseSection.width = mainWindow.width - newsSection.width - 3*mainWindow.spacing
                        }

                    }
                }
            }
        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.top: parent.bottom
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        recentProjectsFrame.height = recentProjectsFrame.height + mouseY
                        exampleProjects.height = exampleProjects.height + mouseY
                        helpFrame.height = helpFrame.height - mouseY
                        datasetFrame.height = datasetFrame.height - mouseY
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.height = 100
                            exampleProjects.height = 100
                            helpFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            recentProjectsFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            exampleProjects.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }
        }

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.right: parent.left
            anchors.leftMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        exampleProjects.width = exampleProjects.width - mouseX
                        recentProjectsFrame.width = recentProjectsFrame.width + mouseX
                        if(recentProjectsFrame.width < 150){
                            recentProjectsFrame.width = 150
                            exampleProjects.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }
                        if(exampleProjects.width < 150) {
                            exampleProjects.width = 150
                            recentProjectsFrame.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }

                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent

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
                Layout.fillWidth: true;
                wrapMode: Text.WordWrap
            }

            TextField {
                id: searchText
                placeholderText: "Search among example projects"
                Layout.fillWidth: true;
                onTextChanged: {exampleGrid.model = helper.searchExampleProjects(searchText.text)}
                height: 25
            }

            GridView {
                id: exampleGrid
                Layout.fillHeight: true
                Layout.fillWidth: true
                cellWidth: width/4
                cellHeight: 150
                ScrollBar.vertical: ScrollBar{}
                clip: true

                model: helper.getExampleProjects();
                delegate: Rectangle {
                    id: exampleDelegate
                    property string name : modelData
                    width: exampleGrid.width
                    height: exampleGrid.height

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        //onEntered: {exampleDelegate.color = '#fdffbf'}
                        //onExited: {exampleDelegate.color = '#ffffff'}
                        onClicked: {mainWindow.openExampleProject(exampleDelegate.name)}
                    }

                    ColumnLayout {
                        Layout.fillHeight: true
                        //Layout.fillWidth: true
                        spacing: 5
                        Image {
                            source: helper.getExampleProjectThumbnail(name)
                            fillMode: Image.Stretch
                            sourceSize.width: Math.min(100, exampleGrid.cellWidth)
                            sourceSize.height: Math.min(100, exampleGrid.cellWidth)
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text {
                            wrapMode: Text.WordWrap
                            text: exampleDelegate.name
                            font.pixelSize: 14
                            font.bold: true
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            wrapMode: Text.WordWrap
                            text: helper.getExampleProjectTags(exampleDelegate.name)
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }
        }
    }

    Frame {
        id: newsSection
        width: mainWindow.width / 5 - 4*mainWindow.spacing
        height: mainWindow.height- 4*mainWindow.spacing
        anchors.bottom: parent.bottom
        anchors.bottomMargin: mainWindow.spacing
        anchors.right: parent.right
        anchors.rightMargin: mainWindow.spacing
        anchors.top: parent.top
        anchors.topMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5
        clip: true

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.right: parent.left
            anchors.leftMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        newsSection.width = newsSection.width - mouseX
                        exampleProjects.width = exampleProjects.width + mouseX
                        datasetFrame.width = datasetFrame.width + mouseX
                        releaseSection.width = releaseSection.width + mouseX
                        if(exampleProjects.width < 150
                                || datasetFrame.width < 150){
                            newsSection.width = mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 150 - 4*mainWindow.spacing
                            exampleProjects.width = mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing
                            releaseSection.width = Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 150

                        }
                        if(newsSection.width < 150) {
                            newsSection.width = 150
                            exampleProjects.width = mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing
                            releaseSection.width = mainWindow.width - newsSection.width - 3*mainWindow.spacing
                        }

                    }
                }
            }
        }

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
                wrapMode: Text.WordWrap
            }

            RssNews{id:newsFeed}
        }

    }

    Frame {
        id: helpFrame
        width: mainWindow.width / 5 - 4*mainWindow.spacing
        height: mainWindow.height / 4 - 4*mainWindow.spacing
        anchors.top: recentProjectsFrame.bottom
        anchors.topMargin: mainWindow.spacing
        anchors.left: parent.left
        anchors.leftMargin: mainWindow.spacing
        anchors.bottom: releaseSection.top
        anchors.bottomMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5
        clip: true

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.left: parent.right
            anchors.rightMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        helpFrame.width = helpFrame.width + mouseX
                        datasetFrame.width = datasetFrame.width - mouseX
                        if(helpFrame.width < 150){
                            helpFrame.width = 150
                            datasetFrame.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }
                        if(datasetFrame.width < 150) {
                            datasetFrame.width = 150
                            helpFrame.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }
                    }
                }
            }

        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.top: parent.bottom
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        helpFrame.height = helpFrame.height + mouseY
                        datasetFrame.height = datasetFrame.height + mouseY
                        releaseSection.height = releaseSection.height - mouseY
                        if(releaseSection.height < 100) {
                            releaseSection.height = 100
                            helpFrame.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            releaseSection.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }

        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.bottom: parent.top
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        recentProjectsFrame.height = recentProjectsFrame.height + mouseY
                        exampleProjects.height = exampleProjects.height + mouseY
                        helpFrame.height = helpFrame.height - mouseY
                        datasetFrame.height = datasetFrame.height - mouseY
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.height = 100
                            exampleProjects.height = 100
                            helpFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            recentProjectsFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            exampleProjects.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: columnLayout2
            anchors.fill: parent
            spacing: 20
            clip: true

            Label {
                id: label3
                text: qsTr("Help")
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 25
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            ListView {
                id: listView2
                x: 0
                y: 0
                width: 110
                height: 160
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
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
                            wrapMode: Text.WordWrap
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
        width: 3 * mainWindow.width / 5 - 4*mainWindow.spacing
        height: mainWindow.height / 4 - 4*mainWindow.spacing
        anchors.top: exampleProjects.bottom
        anchors.topMargin: mainWindow.spacing
        anchors.left: helpFrame.right
        anchors.leftMargin: mainWindow.spacing
        anchors.right: newsSection.left
        anchors.rightMargin: mainWindow.spacing
        anchors.bottom: releaseSection.top
        anchors.bottomMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5
        clip: true

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.left: parent.right
            anchors.rightMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        newsSection.width = newsSection.width - mouseX
                        exampleProjects.width = exampleProjects.width + mouseX
                        datasetFrame.width = datasetFrame.width + mouseX
                        releaseSection.width = releaseSection.width + mouseX
                        if(exampleProjects.width < 150
                                || datasetFrame.width < 150){


                            newsSection.width = mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 150 - 4*mainWindow.spacing
                            exampleProjects.width = mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing
                            releaseSection.width = Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 150

                        }
                        if(newsSection.width < 150) {
                            newsSection.width = 150
                            exampleProjects.width = mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing
                            releaseSection.width = mainWindow.width - newsSection.width - 3*mainWindow.spacing
                        }

                    }
                }
            }
        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.top: parent.bottom
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        helpFrame.height = helpFrame.height + mouseY
                        datasetFrame.height = datasetFrame.height + mouseY
                        releaseSection.height = releaseSection.height - mouseY
                        if(releaseSection.height < 100) {
                            releaseSection.height = 100
                            helpFrame.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            releaseSection.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }
        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.bottom: parent.top
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        recentProjectsFrame.height = recentProjectsFrame.height + mouseY
                        exampleProjects.height = exampleProjects.height + mouseY
                        helpFrame.height = helpFrame.height - mouseY
                        datasetFrame.height = datasetFrame.height - mouseY
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.height = 100
                            exampleProjects.height = 100
                            helpFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            recentProjectsFrame.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                            exampleProjects.height = mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }
        }


        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.right: parent.left
            anchors.leftMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        helpFrame.width = helpFrame.width + mouseX
                        datasetFrame.width = datasetFrame.width - mouseX
                        if(helpFrame.width < 150){
                            helpFrame.width = 150
                            datasetFrame.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }
                        if(datasetFrame.width < 150) {
                            datasetFrame.width = 150
                            helpFrame.width = mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: columnLayout3
            anchors.fill: parent
            clip: true

            Label {
                id: label4
                text: qsTr("Start exploring data")
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 25
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            RowLayout {
                id: rowLayout
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10
                clip: true

                ListView {
                    id: categoryList
                    width: textWidth
                    implicitWidth: textWidth
                    spacing: 10
                    Layout.fillHeight: true
                    property int textWidth: 100
                    clip: true
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
                                wrapMode: Text.WordWrap
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
                    ScrollBar.vertical: ScrollBar{}
                    clip: true
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
                                wrapMode: Text.WordWrap
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
                    clip: true

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
                                wrapMode: Text.WordWrap
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
                        spacing: 10

                        Row {
                            width: datasetDescriptionColumn.width
                            spacing: 5
                            Text {
                                id: datasetTitleLabel
                                text: "Full name: "
                                font.pixelSize: 14
                                font.bold: true
                                wrapMode: Text.WordWrap
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
                                wrapMode: Text.WordWrap
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
                                wrapMode: Text.WordWrap
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
                                wrapMode: Text.WordWrap
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
                                wrapMode: Text.WordWrap
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
        id: releaseSection
        width: 4 * mainWindow.width / 5 - 4*mainWindow.spacing
        height: 2*mainWindow.height / 4 - 4*mainWindow.spacing
        anchors.left: parent.left
        anchors.leftMargin: mainWindow.spacing
        anchors.right: newsSection.left
        anchors.rightMargin: mainWindow.spacing
        anchors.bottom: parent.bottom
        anchors.bottomMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5
        clip: true

        Rectangle {
            width: 3
            height: parent.height
            color: "gray"
            anchors.left: parent.right
            anchors.rightMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.XAxis }
                onMouseXChanged: {
                    if(drag.active){
                        newsSection.width = newsSection.width - mouseX
                        exampleProjects.width = exampleProjects.width + mouseX
                        datasetFrame.width = datasetFrame.width + mouseX
                        releaseSection.width = releaseSection.width + mouseX
                        if(exampleProjects.width < 150
                                || datasetFrame.width < 150){


                            newsSection.width = mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 150 - 4*mainWindow.spacing
                            exampleProjects.width = mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing
                            releaseSection.width = Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 150

                        }
                        if(newsSection.width < 150) {
                            newsSection.width = 150
                            exampleProjects.width = mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing
                            datasetFrame.width = mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing
                            releaseSection.width = mainWindow.width - newsSection.width - 3*mainWindow.spacing
                        }

                    }
                }
            }
        }

        Rectangle {
            height: 3
            width : parent.width
            color: "gray"
            anchors.bottom: parent.top
            anchors.bottomMargin: 0
            opacity: 0

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {parent.opacity = 1}
                onExited: {
                    if(!drag.active && !pressed)
                        parent.opacity = 0
                }
                onPressed:parent.opacity = 1
                onPressAndHold:parent.opacity = 1
                onReleased: parent.opacity = 0
                drag{ target: parent; axis: Drag.YAxis }
                onMouseYChanged: {
                    if(drag.active){
                        helpFrame.height = helpFrame.height + mouseY
                        datasetFrame.height = datasetFrame.height + mouseY
                        releaseSection.height = releaseSection.height - mouseY
                        if(releaseSection.height < 100) {
                            releaseSection.height = 100
                            helpFrame.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                            datasetFrame.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.height = 100
                            datasetFrame.height = 100
                            releaseSection.height = mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: columnLayout7
            anchors.fill: parent

            Label {
                id: label8
                text: qsTr("What's new in this release")
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 30
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
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
