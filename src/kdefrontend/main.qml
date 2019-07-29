import QtQuick.Window 2.12
import QtQuick 2.6
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

    function restoreOriginalLayout() {
        console.log("Restore widget dimensions")
        recentProjectsFrame.widthRate = (mainWindow.width / 5 - 4*mainWindow.spacing)/ mainWindow.width
        recentProjectsFrame.heightRate = (mainWindow.height / 4 - 4*mainWindow.spacing)/ mainWindow.height
        exampleProjects.widthRate = (3 * mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width
        exampleProjects.heightRate =(mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height
        newsSection.widthRate = (mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width
        newsSection.heightRate = (mainWindow.height- 4*mainWindow.spacing) / mainWindow.height
        helpFrame.widthRate = (mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width
        helpFrame.heightRate = (mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height
        datasetFrame.widthRate = (3 * mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width
        datasetFrame.heightRate = (mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height
        releaseSection.widthRate = (4 * mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width
        releaseSection.heightRate = (2*mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height
    }

    function saveWidgetDimensions() {
        console.log("Save welcome screen widget dimensions")
        helper.setHeightScale(recentProjectsFrame.sectionName, recentProjectsFrame.heightRate)
        helper.setWidthScale(recentProjectsFrame.sectionName, recentProjectsFrame.widthRate)
        helper.setHeightScale(exampleProjects.sectionName, exampleProjects.heightRate)
        helper.setWidthScale(exampleProjects.sectionName, exampleProjects.widthRate)
        helper.setHeightScale(newsSection.sectionName, newsSection.heightRate)
        helper.setWidthScale(newsSection.sectionName, newsSection.widthRate)
        helper.setHeightScale(helpFrame.sectionName, helpFrame.heightRate)
        helper.setWidthScale(helpFrame.sectionName, helpFrame.widthRate)
        helper.setHeightScale(datasetFrame.sectionName, datasetFrame.heightRate)
        helper.setWidthScale(datasetFrame.sectionName, datasetFrame.widthRate)
        helper.setHeightScale(releaseSection.sectionName, releaseSection.heightRate)
        helper.setWidthScale(releaseSection.sectionName, releaseSection.widthRate)
    }

    function hideTiles() {
        recentProjectsFrame.visible = false
        recentProjectsFrame.z = 0
        exampleProjects.visible = false
        exampleProjects.z = 0
        newsSection.visible = false
        newsSection.z = 0
        helpFrame.visible = false
        helpFrame.z = 0
        datasetFrame.visible = false
        datasetFrame.z = 0
        releaseSection.visible = false
        releaseSection.z = 0
    }

    function showTiles() {
        recentProjectsFrame.visible = true
        recentProjectsFrame.z = 0
        exampleProjects.visible = true
        exampleProjects.z = 0
        newsSection.visible = true
        newsSection.z = 0
        helpFrame.visible = true
        helpFrame.z = 0
        datasetFrame.visible = true
        datasetFrame.z = 0
        releaseSection.visible = true
        releaseSection.z = 0
    }

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
        property string sectionName: "recentProjectsFrame"
        property double widthRate : helper.getWidthScale(sectionName) === -1 ? (mainWindow.width / 5 - 4*mainWindow.spacing) /  mainWindow.width : helper.getWidthScale(sectionName)
        property double heightRate : helper.getHeightScale(sectionName) === -1 ? (mainWindow.height / 4 - 4*mainWindow.spacing) /  mainWindow.height : helper.getHeightScale(sectionName)
        width: mainWindow.width * widthRate
        height: mainWindow.height * heightRate
        anchors.top: parent.top
        anchors.topMargin: mainWindow.spacing
        anchors.left: parent.left
        anchors.leftMargin: mainWindow.spacing
        visible: true
        opacity: 1
        padding: 5
        property bool fullScreen: false
        property double prevWidth: 0
        property double prevHeight: 0

        Component.onCompleted: {
            console.log("Recent projects saved iwdth " + helper.getWidthScale(sectionName) + "  height  " + helper.getHeightScale(sectionName))
            if(helper.getWidthScale(sectionName) === -1 || helper.getHeightScale(sectionName) === -1)
                mainWindow.restoreOriginalLayout()
        }

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
                        recentProjectsFrame.widthRate = (recentProjectsFrame.width + mouseX) / mainWindow.width
                        exampleProjects.widthRate = (exampleProjects.width - mouseX) / mainWindow.width
                        if(recentProjectsFrame.width < 150){
                            recentProjectsFrame.widthRate = 150 / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                        }
                        if(exampleProjects.width < 300) {
                            exampleProjects.widthRate = 300 / mainWindow.width
                            recentProjectsFrame.widthRate = (mainWindow.width - newsSection.width - 300 - 4*mainWindow.spacing) / mainWindow.width
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
                        recentProjectsFrame.heightRate = (recentProjectsFrame.height + mouseY) / mainWindow.height
                        exampleProjects.heightRate = (exampleProjects.height + mouseY) / mainWindow.height
                        helpFrame.heightRate = (helpFrame.height - mouseY) / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height - mouseY) / mainWindow.height
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.heightRate = 100 / mainWindow.height
                            exampleProjects.heightRate = 100 / mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            recentProjectsFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            exampleProjects.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                    }
                }
            }

        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 20
            clip: true

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: Math.min(parent.height*0.2, 100)
                Layout.preferredHeight: Math.min(parent.height*0.2, 100)

                Image {
                    Layout.preferredHeight: recentProjectsFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumHeight: recentProjectsFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.preferredWidth: recentProjectsFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumWidth: recentProjectsFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.alignment: Qt.AlignVCenter

                    source: recentProjectsFrame.fullScreen ? helper.getMinIcon() : helper.getMaxIcon()
                    sourceSize.width: recentProjectsFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    sourceSize.height: recentProjectsFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)


                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if(!recentProjectsFrame.fullScreen) {
                                hideTiles()
                                recentProjectsFrame.prevWidth = recentProjectsFrame.widthRate
                                recentProjectsFrame.prevHeight = recentProjectsFrame.heightRate
                                recentProjectsFrame.visible = true
                                recentProjectsFrame.z = 1
                                recentProjectsFrame.anchors.fill = undefined
                                recentProjectsFrame.anchors.right = undefined
                                recentProjectsFrame.anchors.bottom = undefined
                                recentProjectsFrame.anchors.centerIn = undefined
                                recentProjectsFrame.anchors.top = undefined
                                recentProjectsFrame.anchors.left = undefined
                                recentProjectsFrame.widthRate = 1
                                recentProjectsFrame.heightRate = 1
                                recentProjectsFrame.anchors.fill = mainWindow
                            } else {
                                recentProjectsFrame.anchors.fill = undefined
                                recentProjectsFrame.anchors.right = undefined
                                recentProjectsFrame.anchors.bottom = undefined
                                recentProjectsFrame.anchors.centerIn = undefined
                                recentProjectsFrame.anchors.top = undefined
                                recentProjectsFrame.anchors.left = undefined
                                recentProjectsFrame.anchors.top = mainWindow.top
                                recentProjectsFrame.anchors.topMargin = mainWindow.spacing
                                recentProjectsFrame.anchors.left = mainWindow.left
                                recentProjectsFrame.anchors.leftMargin = mainWindow.spacing
                                recentProjectsFrame.widthRate = recentProjectsFrame.prevWidth
                                recentProjectsFrame.heightRate = recentProjectsFrame.prevHeight

                                showTiles();
                            }

                            recentProjectsFrame.fullScreen = !recentProjectsFrame.fullScreen
                        }
                    }
                }


                Label {
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
                    font.pointSize: recentProjectsFrame.fullScreen ? 60 : 24
                    minimumPointSize: 10
                    fontSizeMode: Text.Fit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                }
            }

            RecentProjects {
                id: recentProjectsList
                model:recentProjects
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: parent.height*0.8
                Layout.preferredHeight: parent.height*0.8
                clip: true
            }
        }
    }

    Frame {
        id: exampleProjects
        property string sectionName: "exampleProjects"
        property double widthRate : helper.getWidthScale(sectionName) === -1 ? (3 * mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width : helper.getWidthScale(sectionName)
        property double heightRate : helper.getHeightScale(sectionName) === -1 ? (mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height : helper.getHeightScale(sectionName)
        width: mainWindow.width * widthRate
        height: mainWindow.height * heightRate
        anchors.top: parent.top
        anchors.topMargin: mainWindow.spacing
        anchors.left: recentProjectsFrame.right
        anchors.leftMargin: mainWindow.spacing
        anchors.right: newsSection.left
        anchors.rightMargin: mainWindow.spacing
        visible: true
        clip: true
        opacity: 1
        padding: 5

        property bool fullScreen: false
        property double prevWidth: 0
        property double prevHeight: 0

        Component.onCompleted: {
            if(helper.getWidthScale(sectionName) === -1 || helper.getHeightScale(sectionName) === -1)
                mainWindow.restoreOriginalLayout()
        }

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
                        newsSection.widthRate = (newsSection.width - mouseX) / mainWindow.width
                        exampleProjects.widthRate = (exampleProjects.width + mouseX) / mainWindow.width
                        datasetFrame.widthRate = (datasetFrame.width + mouseX) / mainWindow.width
                        releaseSection.widthRate = (releaseSection.width + mouseX) / mainWindow.width
                        if(exampleProjects.width < 300
                                || datasetFrame.width < 300){


                            newsSection.widthRate = (mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 300 - 4*mainWindow.spacing) / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing) / mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 300) / mainWindow.width

                        }
                        if(newsSection.width < 150) {
                            newsSection.widthRate = 150 / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                            datasetFrame.widthRate =(mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (mainWindow.width - newsSection.width - 3*mainWindow.spacing) / mainWindow.width
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
                        recentProjectsFrame.heightRate = (recentProjectsFrame.height + mouseY) / mainWindow.height
                        exampleProjects.heightRate = (exampleProjects.height + mouseY) / mainWindow.height
                        helpFrame.heightRate = (helpFrame.height - mouseY) / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height - mouseY) / mainWindow.height
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.heightRate = 100 / mainWindow.height
                            exampleProjects.heightRate = 100/ mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            recentProjectsFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            exampleProjects.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing)/ mainWindow.height
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
                        exampleProjects.widthRate = (exampleProjects.width - mouseX)/ mainWindow.width
                        recentProjectsFrame.widthRate = (recentProjectsFrame.width + mouseX) / mainWindow.width
                        if(recentProjectsFrame.width < 150){
                            recentProjectsFrame.widthRate = 150 / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                        }
                        if(exampleProjects.width < 300) {
                            exampleProjects.widthRate = 300/ mainWindow.width
                            recentProjectsFrame.widthRate = (mainWindow.width - newsSection.width - 300 - 4*mainWindow.spacing) / mainWindow.width
                        }

                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            clip: true
            spacing: 5

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: Math.min((parent.height - 25 - 2*parent.spacing)*0.2, 100)
                Layout.preferredHeight: Math.min((parent.height - 25 - 2*parent.spacing)*0.2, 100)

                Image {
                    Layout.preferredHeight: exampleProjects.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumHeight: exampleProjects.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.preferredWidth: exampleProjects.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumWidth: exampleProjects.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.alignment: Qt.AlignVCenter

                    source: exampleProjects.fullScreen ? helper.getMinIcon() : helper.getMaxIcon()
                    sourceSize.width: exampleProjects.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    sourceSize.height: exampleProjects.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)


                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if(!exampleProjects.fullScreen) {
                                hideTiles()
                                exampleProjects.prevWidth = exampleProjects.widthRate
                                exampleProjects.prevHeight = exampleProjects.heightRate
                                exampleProjects.visible = true
                                exampleProjects.z = 1
                                exampleProjects.anchors.fill = undefined
                                exampleProjects.anchors.right = undefined
                                exampleProjects.anchors.bottom = undefined
                                exampleProjects.anchors.centerIn = undefined
                                exampleProjects.anchors.top = undefined
                                exampleProjects.anchors.left = undefined

                                exampleProjects.widthRate = 1
                                exampleProjects.heightRate = 1

                                exampleProjects.anchors.fill = mainWindow
                            } else {
                                exampleProjects.anchors.fill = undefined
                                exampleProjects.anchors.right = undefined
                                exampleProjects.anchors.bottom = undefined
                                exampleProjects.anchors.centerIn = undefined
                                exampleProjects.anchors.top = undefined
                                exampleProjects.anchors.left = undefined

                                exampleProjects.anchors.top = mainWindow.top
                                exampleProjects.anchors.topMargin = mainWindow.spacing
                                exampleProjects.anchors.left = recentProjectsFrame.right
                                exampleProjects.anchors.leftMargin = mainWindow.spacing
                                exampleProjects.anchors.right = newsSection.left
                                exampleProjects.anchors.rightMargin = mainWindow.spacing
                                exampleProjects.widthRate = exampleProjects.prevWidth
                                exampleProjects.heightRate = exampleProjects.prevHeight

                                showTiles();
                            }

                            exampleProjects.fullScreen = !exampleProjects.fullScreen
                        }
                    }
                }


                Label {
                    text: qsTr("Examples")
                    styleColor: "#d41919"

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: exampleProjects.fullScreen ? 60 : 24
                    minimumPointSize: 10
                    fontSizeMode: Text.Fit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                }
            }

            TextField {
                id: searchText
                placeholderText: "Search among example projects"
                Layout.fillWidth: true;
                onTextChanged: {exampleGrid.model = helper.searchExampleProjects(searchText.text)}
                height: 25
                Layout.minimumHeight: 25
                Layout.preferredHeight: 25
            }

            ExampleGrid {
                id: exampleGrid
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip:true

                Layout.minimumHeight: Math.max((parent.height - 25 - 2*parent.spacing)*0.8, parent.height - 25 - 100  - 2*parent.spacing)
                Layout.preferredHeight: Math.max((parent.height - 25 - 2*parent.spacing)*0.8, parent.height - 25 - 100  - 2*parent.spacing)
            }
        }
    }

    Frame {
        id: newsSection
        property string sectionName: "newsSection"
        property double widthRate : helper.getWidthScale(sectionName) === -1 ? (mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width : helper.getWidthScale(sectionName)
        property double heightRate : helper.getHeightScale(sectionName) === -1 ? (mainWindow.height- 4*mainWindow.spacing) / mainWindow.height : helper.getHeightScale(sectionName)
        width: mainWindow.width * widthRate
        height: mainWindow.height * heightRate

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

        property bool fullScreen: false
        property double prevWidth: 0
        property double prevHeight: 0

        Component.onCompleted: {
            if(helper.getWidthScale(sectionName) === -1 || helper.getHeightScale(sectionName) === -1)
                mainWindow.restoreOriginalLayout()
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
                        newsSection.widthRate = (newsSection.width - mouseX) / mainWindow.width
                        exampleProjects.widthRate = (exampleProjects.width + mouseX) / mainWindow.width
                        datasetFrame.widthRate = (datasetFrame.width + mouseX) / mainWindow.width
                        releaseSection.widthRate = (releaseSection.width + mouseX) / mainWindow.width
                        if(exampleProjects.width < 300
                                || datasetFrame.width < 300){
                            newsSection.widthRate = (mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 300 - 4*mainWindow.spacing) / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing)/ mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 300) / mainWindow.width

                        }
                        if(newsSection.width < 150) {
                            newsSection.widthRate = 150 / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (mainWindow.width - newsSection.width - 3*mainWindow.spacing) / mainWindow.width
                        }

                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: Math.min(parent.height*0.2, 100)
                Layout.preferredHeight: Math.min(parent.height*0.2, 100)

                Image {
                    Layout.preferredHeight: newsSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumHeight: newsSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.preferredWidth: newsSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumWidth:newsSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.alignment: Qt.AlignVCenter

                    source: newsSection.fullScreen ? helper.getMinIcon() : helper.getMaxIcon()
                    sourceSize.width: newsSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    sourceSize.height: newsSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)


                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if(!newsSection.fullScreen) {
                                hideTiles()
                                newsSection.prevWidth = newsSection.widthRate
                                newsSection.prevHeight = newsSection.heightRate
                                newsSection.visible = true
                                newsSection.z = 1
                                newsSection.anchors.fill = undefined
                                newsSection.anchors.right = undefined
                                newsSection.anchors.bottom = undefined
                                newsSection.anchors.centerIn = undefined
                                newsSection.anchors.top = undefined
                                newsSection.anchors.left = undefined


                                newsSection.widthRate = 1
                                newsSection.heightRate = 1

                                newsSection.anchors.fill = mainWindow
                            } else {
                                newsSection.anchors.fill = undefined
                                newsSection.anchors.right = undefined
                                newsSection.anchors.bottom = undefined
                                newsSection.anchors.centerIn = undefined
                                newsSection.anchors.top = undefined
                                newsSection.anchors.left = undefined

                                newsSection.anchors.top = mainWindow.top
                                newsSection.anchors.topMargin = mainWindow.spacing
                                newsSection.anchors.right = mainWindow.right
                                newsSection.anchors.rightMargin = mainWindow.spacing
                                newsSection.anchors.bottom = mainWindow.bottom
                                newsSection.anchors.bottomMargin = mainWindow.spacing
                                newsSection.widthRate = newsSection.prevWidth
                                newsSection.heightRate = newsSection.prevHeight

                                showTiles();
                            }

                            newsSection.fullScreen = !newsSection.fullScreen
                        }
                    }
                }

                Label {
                    id: label2
                    text: qsTr("News")

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: newsSection.fullScreen ? 60 : 24
                    minimumPointSize: 10
                    fontSizeMode: Text.Fit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                }
            }

            RssNews{
                id:newsFeed
                Layout.fillHeight: true
                Layout.minimumHeight: Math.min(parent.height * 0.8, parent.height - 100)
            }

        }

    }

    Frame {
        id: helpFrame
        property string sectionName: "helpFrame"
        property double widthRate : helper.getWidthScale(sectionName) === -1 ? (mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width : helper.getWidthScale(sectionName)
        property double heightRate : helper.getHeightScale(sectionName) === -1 ? (mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height : helper.getHeightScale(sectionName)
        width: mainWindow.width * widthRate
        height: mainWindow.height * heightRate

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

        property bool fullScreen: false
        property double prevWidth: 0
        property double prevHeight: 0

        Component.onCompleted: {
            if(helper.getWidthScale(sectionName) === -1 || helper.getHeightScale(sectionName) === -1)
                mainWindow.restoreOriginalLayout()
        }

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
                        helpFrame.widthRate = (helpFrame.width + mouseX) / mainWindow.width
                        datasetFrame.widthRate = (datasetFrame.width - mouseX) / mainWindow.width
                        if(helpFrame.width < 150){
                            helpFrame.widthRate = 150 / mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                        }
                        if(datasetFrame.width < 150) {
                            datasetFrame.widthRate = 150 / mainWindow.width
                            helpFrame.widthRate = (mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing) / mainWindow.width
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
                        helpFrame.heightRate = (helpFrame.height + mouseY) / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height + mouseY) / mainWindow.height
                        releaseSection.heightRate = (releaseSection.height - mouseY) / mainWindow.height
                        if(releaseSection.height < 100) {
                            releaseSection.heightRate = 100 / mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            releaseSection.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
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
                        recentProjectsFrame.heightRate = (recentProjectsFrame.height + mouseY) / mainWindow.height
                        exampleProjects.heightRate =(exampleProjects.height + mouseY) / mainWindow.height
                        helpFrame.heightRate = (helpFrame.height - mouseY)  / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height - mouseY) / mainWindow.height
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.heightRate = 100 /  mainWindow.height
                            exampleProjects.heightRate = 100/ mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing)  / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            recentProjectsFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) /  mainWindow.height
                            exampleProjects.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 20
            clip: true

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: Math.min((parent.height - parent.spacing) *0.2, 100)
                Layout.preferredHeight: Math.min((parent.height - parent.spacing) *0.2, 100)

                Image {
                    Layout.preferredHeight: helpFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumHeight: helpFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.preferredWidth: helpFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumWidth: helpFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.alignment: Qt.AlignVCenter

                    source: helpFrame.fullScreen ? helper.getMinIcon() : helper.getMaxIcon()
                    sourceSize.width: helpFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    sourceSize.height: helpFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)


                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if(!helpFrame.fullScreen) {
                                hideTiles()
                                helpFrame.prevWidth = helpFrame.widthRate
                                helpFrame.prevHeight = helpFrame.heightRate
                                helpFrame.visible = true
                                helpFrame.z = 1
                                helpFrame.anchors.fill = undefined
                                helpFrame.anchors.right = undefined
                                helpFrame.anchors.bottom = undefined
                                helpFrame.anchors.centerIn = undefined
                                helpFrame.anchors.top = undefined
                                helpFrame.anchors.left = undefined

                                helpFrame.heightRate = 1
                                helpFrame.widthRate = 1
                                helpFrame.anchors.fill = mainWindow
                            } else {
                                helpFrame.anchors.fill = undefined
                                helpFrame.anchors.right = undefined
                                helpFrame.anchors.bottom = undefined
                                helpFrame.anchors.centerIn = undefined
                                helpFrame.anchors.top = undefined
                                helpFrame.anchors.left = undefined

                                helpFrame.anchors.top = recentProjectsFrame.bottom
                                helpFrame.anchors.topMargin = mainWindow.spacing
                                helpFrame.anchors.left = mainWindow.left
                                helpFrame.anchors.leftMargin = mainWindow.spacing
                                helpFrame.anchors.bottom = releaseSection.top
                                helpFrame.anchors.bottomMargin = mainWindow.spacing
                                helpFrame.widthRate = helpFrame.prevWidth
                                helpFrame.heightRate = helpFrame.prevHeight

                                showTiles();
                            }

                            helpFrame.fullScreen = !helpFrame.fullScreen
                        }
                    }
                }

                Label {
                    text: qsTr("Help")

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: helpFrame.fullScreen ? 60 : 24
                    minimumPointSize: 10
                    fontSizeMode: Text.Fit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                }
            }

            HelpList {
                id: helpList
                width: parent.width
                Layout.minimumHeight: Math.max((parent.height - parent.spacing) *0.8, parent.height - parent.spacing - 100)
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
            }
        }
    }

    Frame {
        id: datasetFrame
        property string sectionName: "datasetFrame"
        property double widthRate : helper.getWidthScale(sectionName) === -1 ? (3 * mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width : helper.getWidthScale(sectionName)
        property double heightRate : helper.getHeightScale(sectionName) === -1 ? (mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height : helper.getHeightScale(sectionName)
        width: mainWindow.width * widthRate
        height: mainWindow.height * heightRate

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

        property bool fullScreen: false
        property double prevWidth: 0
        property double prevHeight: 0

        Component.onCompleted: {
            if(helper.getWidthScale(sectionName) === -1 || helper.getHeightScale(sectionName) === -1)
                mainWindow.restoreOriginalLayout()
        }

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
                        newsSection.widthRate = (newsSection.width - mouseX) / mainWindow.width
                        exampleProjects.widthRate = (exampleProjects.width + mouseX) / mainWindow.width
                        datasetFrame.widthRate = (datasetFrame.width + mouseX) / mainWindow.width
                        releaseSection.widthRate = (releaseSection.width + mouseX) / mainWindow.width
                        if(exampleProjects.width < 300
                                || datasetFrame.width < 300){


                            newsSection.widthRate = (mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 300 - 4*mainWindow.spacing) / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing)/ mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 300) / mainWindow.width

                        }
                        if(newsSection.width < 150) {
                            newsSection.widthRate = 150 / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing)/ mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (mainWindow.width - newsSection.width - 3*mainWindow.spacing) / mainWindow.width
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
                        helpFrame.heightRate = (helpFrame.height + mouseY) / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height + mouseY) / mainWindow.height
                        releaseSection.heightRate = (releaseSection.height - mouseY) / mainWindow.height
                        if(releaseSection.height < 100) {
                            releaseSection.heightRate = 100 / mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            releaseSection.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
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
                        recentProjectsFrame.heightRate = (recentProjectsFrame.height + mouseY) / mainWindow.height
                        exampleProjects.heightRate = (exampleProjects.height + mouseY) / mainWindow.height
                        helpFrame.heightRate = (helpFrame.height - mouseY) / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height - mouseY) / mainWindow.height
                        if(recentProjectsFrame.height < 100 || exampleProjects.height < 100) {
                            recentProjectsFrame.heightRate = 100 /  mainWindow.height
                            exampleProjects.heightRate = 100 / mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            recentProjectsFrame.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) /  mainWindow.height
                            exampleProjects.heightRate = (mainWindow.height - releaseSection.height - 100 - 4*mainWindow.spacing) / mainWindow.height
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
                        helpFrame.widthRate = (helpFrame.width + mouseX) / mainWindow.width
                        datasetFrame.widthRate = (datasetFrame.width - mouseX) / mainWindow.width
                        if(helpFrame.width < 150){
                            helpFrame.widthRate = 150 / mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                        }
                        if(datasetFrame.width < 150) {
                            datasetFrame.widthRate = 150 / mainWindow.width
                            helpFrame.widthRate = (mainWindow.width - newsSection.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                        }
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            clip: true
            spacing: 20

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: Math.min((parent.height - parent.spacing) *0.2, 100)
                Layout.preferredHeight: Math.min((parent.height - parent.spacing) *0.2, 100)

                Image {
                    Layout.preferredHeight: datasetFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumHeight: datasetFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.preferredWidth: datasetFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumWidth: datasetFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.alignment: Qt.AlignVCenter

                    source: datasetFrame.fullScreen ? helper.getMinIcon() : helper.getMaxIcon()
                    sourceSize.width: datasetFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    sourceSize.height: datasetFrame.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if(!datasetFrame.fullScreen) {
                                hideTiles()
                                datasetFrame.prevWidth = datasetFrame.widthRate
                                datasetFrame.prevHeight = datasetFrame.heightRate
                                datasetFrame.visible = true
                                datasetFrame.z = 1
                                datasetFrame.anchors.fill = undefined
                                datasetFrame.anchors.right = undefined
                                datasetFrame.anchors.bottom = undefined
                                datasetFrame.anchors.centerIn = undefined
                                datasetFrame.anchors.top = undefined
                                datasetFrame.anchors.left = undefined

                                datasetFrame.widthRate = 1
                                datasetFrame.heightRate = 1
                                datasetFrame.anchors.fill = mainWindow
                            } else {
                                datasetFrame.anchors.fill = undefined
                                datasetFrame.anchors.right = undefined
                                datasetFrame.anchors.bottom = undefined
                                datasetFrame.anchors.centerIn = undefined
                                datasetFrame.anchors.top = undefined
                                datasetFrame.anchors.left = undefined

                                datasetFrame.anchors.top = exampleProjects.bottom
                                datasetFrame.anchors.topMargin = mainWindow.spacing
                                datasetFrame.anchors.left = helpFrame.right
                                datasetFrame.anchors.leftMargin = mainWindow.spacing
                                datasetFrame.anchors.bottom = releaseSection.top
                                datasetFrame.anchors.bottomMargin = mainWindow.spacing
                                datasetFrame.anchors.right = newsSection.left
                                datasetFrame.anchors.rightMargin = mainWindow.spacing
                                datasetFrame.widthRate = datasetFrame.prevWidth
                                datasetFrame.heightRate = datasetFrame.prevHeight

                                showTiles();
                            }

                            datasetFrame.fullScreen = !datasetFrame.fullScreen
                        }
                    }
                }

                Label {
                    text: qsTr("Start exploring data")

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: datasetFrame.fullScreen ? 60 : 24
                    minimumPointSize: 10
                    fontSizeMode: Text.Fit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                }
            }

            RowLayout {
                id: datasetSectionRow
                Layout.fillHeight: true
                Layout.minimumHeight: Math.max((parent.height - parent.spacing) *0.8, parent.height - parent.spacing - 100)
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                Layout.minimumWidth: parent.width
                spacing: 10
                clip: true
                property int separatorWidth: 5

                ListView {
                    id: categoryList
                    Layout.preferredWidth: (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.15
                    Layout.minimumWidth:  (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.15
                    //width: textWidth
                    // implicitWidth: textWidth
                    spacing: 10
                    Layout.fillHeight: true
                    property int textWidth: 100

                    clip: true
                    ScrollBar.vertical: ScrollBar { }
                    Component.onCompleted: console.log("Model: " +  datasetModel.allCategories())
                    model: datasetModel.allCategories()
                    delegate:Rectangle {
                        width: parent.width
                        height: 25
                        id: categoryDelegate
                        property string categoryName : modelData
                        property bool selected: ListView.isCurrentItem

                        RowLayout {
                            id: categoryRow
                            spacing: 10
                            //Layout.fillHeight: true
                            //Layout.fillWidth: true
                            anchors.fill: parent

                            Rectangle {
                                id: categoryBullet
                                anchors.verticalCenter: parent.verticalCenter
                                width: 5
                                height: 5
                                color: "#7a7d82"
                            }

                            Label {
                                height: parent.height
                                width: parent.width - 5 - categoryRow.spacing
                                Layout.minimumWidth: parent.width - 5 - categoryRow.spacing
                                Layout.preferredWidth: parent.width - 5 - categoryRow.spacing
                                Layout.fillHeight: true
                                id: categoryLabel
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                text: categoryDelegate.categoryName
                                font.bold: true
                                wrapMode: Text.WordWrap
                                font.pixelSize: 18
                                minimumPixelSize: 1
                                fontSizeMode: Text.Fit
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
                    spacing: 10
                    Layout.fillHeight: true
                    Layout.preferredWidth: (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.15
                    Layout.minimumWidth:  (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.15
                    ScrollBar.vertical: ScrollBar{}
                    clip: true
                    property int textWidth: 100

                    model: datasetModel.allSubcategories(mainWindow.currentCategory)
                    delegate: Rectangle {
                        width: parent.width
                        height: 25
                        id: subcategoryDelegate
                        property string subcategoryName : modelData
                        property bool selected: ListView.isCurrentItem

                        RowLayout {
                            id: subcategoryRow
                            spacing: 10
                            anchors.fill: parent

                            Rectangle {
                                id: subcategoryBullet
                                anchors.verticalCenter: parent.verticalCenter
                                width: 5
                                height: 5
                                color: "#7a7d82"
                            }

                            Label {
                                height: parent.height
                                width: parent.width - 5 - subcategoryRow.spacing
                                Layout.minimumWidth: parent.width - 5 - subcategoryRow.spacing
                                Layout.preferredWidth: parent.width - 5 - subcategoryRow.spacing
                                Layout.fillHeight: true

                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                text: subcategoryDelegate.subcategoryName
                                wrapMode: Text.WordWrap
                                font.bold: true
                                font.pixelSize: 18
                                minimumPixelSize: 1
                                fontSizeMode: Text.Fit
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
                    Layout.preferredWidth: (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.4
                    Layout.minimumWidth:  (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.4
                    cellWidth: width/4
                    cellHeight: 40
                    clip: true


                    model: datasetModel.allDatasets(mainWindow.currentCategory, mainWindow.currentSubcategory)

                    delegate: Rectangle {
                        id: datasetDelegate
                        property string datasetName : modelData
                        property bool selected: (index == datasetGrid.currentIndex)
                        width: (datasetGrid.width - 30) / 4
                        height: textHeight

                        property int textWidth: 200
                        property int textHeight: 40

                        RowLayout {
                            id: datasetRow
                            spacing: 10
                            anchors.fill: parent

                            Rectangle {
                                id: datasetBullet
                                width: 5
                                height: 5
                                color: "#7a7d82"
                            }

                            Label {
                                id: datasetText


                                height: parent.height
                                width: parent.width - 5 - datasetRow.spacing
                                Layout.minimumWidth: parent.width - 5 - datasetRow.spacing
                                Layout.preferredWidth: parent.width - 5 - datasetRow.spacing
                                Layout.fillHeight: true

                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                text: datasetDelegate.datasetName
                                wrapMode: Text.WordWrap
                                font.bold: true
                                font.pixelSize: 18
                                minimumPixelSize: 1
                                fontSizeMode: Text.Fit
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
                    id: scrollView
                    Layout.fillHeight: true
                    Layout.preferredWidth: (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.3
                    Layout.minimumWidth:  (parent.width - 3 * datasetSectionRow.separatorWidth - 6*datasetSectionRow.spacing) * 0.3

                    contentHeight: datasetDescriptionColumn.height
                    clip: true
                    ColumnLayout {
                        id: datasetDescriptionColumn
                        //anchors.fill: parent
                        Layout.minimumWidth: scrollView.width // datasetFrame.width *0.3
                        Layout.preferredWidth: scrollView.width //datasetFrame.width *0.3
                        width: scrollView.width //datasetFrame.width *0.3
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
        property string sectionName: "releaseSection"
        property double widthRate : helper.getWidthScale(sectionName) === -1 ? (4 * mainWindow.width / 5 - 4*mainWindow.spacing) / mainWindow.width : helper.getWidthScale(sectionName)
        property double heightRate : helper.getHeightScale(sectionName) === -1 ? (2*mainWindow.height / 4 - 4*mainWindow.spacing) / mainWindow.height : helper.getHeightScale(sectionName)
        width: mainWindow.width * widthRate
        height: mainWindow.height * heightRate

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

        property bool fullScreen: false
        property double prevWidth: 0
        property double prevHeight: 0


        Component.onCompleted: {
            if(helper.getWidthScale(sectionName) === -1 || helper.getHeightScale(sectionName) === -1)
                mainWindow.restoreOriginalLayout()
        }

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
                        newsSection.widthRate = (newsSection.width - mouseX) / mainWindow.width
                        exampleProjects.widthRate = (exampleProjects.width + mouseX) / mainWindow.width
                        datasetFrame.widthRate = (datasetFrame.width + mouseX) / mainWindow.width
                        releaseSection.widthRate = (releaseSection.width + mouseX) / mainWindow.width
                        if(exampleProjects.width < 300
                                || datasetFrame.width < 300){


                            newsSection.widthRate = (mainWindow.width - Math.max(recentProjectsFrame.width, helpFrame.width) - 300 - 4*mainWindow.spacing) / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - newsSection.width - recentProjectsFrame.width - 4*mainWindow.spacing)  / mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - newsSection.width - helpFrame.width - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (Math.max(recentProjectsFrame.width, helpFrame.width) + mainWindow.spacing + 300) / mainWindow.width

                        }
                        if(newsSection.width < 150) {
                            newsSection.width = 150 / mainWindow.width
                            exampleProjects.widthRate = (mainWindow.width - recentProjectsFrame.width - 150 - 4*mainWindow.spacing)  / mainWindow.width
                            datasetFrame.widthRate = (mainWindow.width - helpFrame.width - 150 - 4*mainWindow.spacing) / mainWindow.width
                            releaseSection.widthRate = (mainWindow.width - newsSection.width - 3*mainWindow.spacing) / mainWindow.width
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
                        helpFrame.heightRate = (helpFrame.height + mouseY) / mainWindow.height
                        datasetFrame.heightRate = (datasetFrame.height + mouseY) / mainWindow.height
                        releaseSection.heightRate = (releaseSection.height - mouseY) / mainWindow.height
                        if(releaseSection.height < 100) {
                            releaseSection.heightRate = 100 / mainWindow.height
                            helpFrame.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing)  / mainWindow.height
                            datasetFrame.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                        if(helpFrame.height < 100 || datasetFrame.height < 100) {
                            helpFrame.heightRate = 100 / mainWindow.height
                            datasetFrame.heightRate = 100 / mainWindow.height
                            releaseSection.heightRate = (mainWindow.height - recentProjectsFrame.height - 100 - 4*mainWindow.spacing) / mainWindow.height
                        }
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            clip: true
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: Math.min((parent.height - parent.spacing) *0.2, 100)
                Layout.preferredHeight: Math.min((parent.height - parent.spacing) *0.2, 100)

                Image {
                    Layout.preferredHeight: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumHeight: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.preferredWidth: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.minimumWidth: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    Layout.alignment: Qt.AlignVCenter
                    source: releaseSection.fullScreen ? helper.getMinIcon() : helper.getMaxIcon()
                    sourceSize.width: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)
                    sourceSize.height: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 25) : Math.min(Math.min(parent.height, parent.width) * 0.5, 15)

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if(!releaseSection.fullScreen) {
                                hideTiles()
                                releaseSection.prevWidth = releaseSection.widthRate
                                releaseSection.prevHeight = releaseSection.heightRate
                                releaseSection.visible = true
                                releaseSection.z = 1
                                releaseSection.anchors.fill = undefined
                                releaseSection.anchors.right = undefined
                                releaseSection.anchors.bottom = undefined
                                releaseSection.anchors.centerIn = undefined
                                releaseSection.anchors.top = undefined
                                releaseSection.anchors.left = undefined

                                releaseSection.widthRate = 1
                                releaseSection.heightRate = 1
                                releaseSection.anchors.fill = mainWindow
                            } else {
                                releaseSection.anchors.fill = undefined
                                releaseSection.anchors.right = undefined
                                releaseSection.anchors.bottom = undefined
                                releaseSection.anchors.centerIn = undefined
                                releaseSection.anchors.top = undefined
                                releaseSection.anchors.left = undefined

                                releaseSection.anchors.left = mainWindow.left
                                releaseSection.anchors.leftMargin = mainWindow.spacing
                                releaseSection.anchors.bottom = mainWindow.bottom
                                releaseSection.anchors.bottomMargin = mainWindow.spacing
                                releaseSection.anchors.right = newsSection.left
                                releaseSection.anchors.rightMargin = mainWindow.spacing
                                releaseSection.widthRate = releaseSection.prevWidth
                                releaseSection.heightRate = releaseSection.prevHeight

                                showTiles();
                            }

                            releaseSection.fullScreen = !releaseSection.fullScreen
                        }
                    }
                }

                Image {
                    Layout.preferredHeight: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.minimumHeight: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.preferredWidth: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.minimumWidth: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.alignment: Qt.AlignVCenter
                    source: helper.getBackIcon()
                    sourceSize.width: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    sourceSize.height: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            releaseWebView.goBack()
                        }
                    }
                }

                Image {
                    Layout.preferredHeight: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.minimumHeight: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.preferredWidth: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.minimumWidth: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    Layout.alignment: Qt.AlignVCenter
                    source: helper.getForwardIcon()
                    sourceSize.width: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)
                    sourceSize.height: releaseSection.fullScreen ?Math.min(Math.min(parent.height, parent.width) * 0.5, 35) : Math.min(Math.min(parent.height, parent.width) * 0.5, 25)

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            releaseWebView.goForward()
                        }
                    }
                }

                Label {
                    text: qsTr("What's new in this release")

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize:releaseSection.fullScreen ? 60 : 24
                    minimumPointSize: 10
                    fontSizeMode: Text.Fit
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                }

            }

            WebView {
                id: releaseWebView
                Layout.fillHeight: true
                Layout.minimumHeight: Math.max((parent.height - parent.spacing) *0.8, parent.height - parent.spacing - 100)
                Layout.preferredHeight: Math.max((parent.height - parent.spacing) *0.8, parent.height - parent.spacing - 100)
                Layout.fillWidth: true
                url: initialUrl
            }
        }
    }
}
