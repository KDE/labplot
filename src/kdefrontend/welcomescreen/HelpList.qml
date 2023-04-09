import QtQuick 2.6
import QtQuick.XmlListModel 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

ListView {
                id: listView2
                width: parent.width
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                ScrollBar.vertical: ScrollBar { }
                model: ListModel {
                    ListElement {
                        name: "Documentation"
                        link: "https://docs.kde.org/?application=labplot2"
                    }

                    ListElement {
                        name: "FAQ"
                        link: "https://docs.kde.org/?application=labplot2&branch=trunk5&path=faq.html"
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
                        height: parent.height
                        spacing: 10

                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            width: 5
                            height: 5
                            color: "#7a7d82"
                        }

                        Label {
                            height: parent.height
                            width: parent.width - 5 - parent.spacing
                            Layout.minimumWidth: parent.width - 5 - parent.spacing
                            Layout.preferredWidth: parent.width - 5 - parent.spacing
                            text: name
                            font.bold: true
                            font.pixelSize: 18
                            minimumPixelSize: 1
                            fontSizeMode: Text.Fit

                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            //wrapMode: Text.WordWrap
                        }
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
