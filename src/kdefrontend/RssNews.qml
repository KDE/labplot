import QtQuick 2.6
import QtQuick.XmlListModel 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

Row {
    id: newsfeed

    Layout.fillHeight: true
    Layout.fillWidth: true

    XmlListModel {
        id: feedModel

        source: "https://labplot.kde.org/feed"
        query: "/rss/channel/item"

        XmlRole { name: "title"; query: "title/string()" }
        // Remove any links from the description
        XmlRole { name: "description"; query: "fn:replace(description/string(), '\&lt;a href=.*\/a\&gt;', '')" }
        XmlRole { name: "link"; query: "link/string()" }
        XmlRole { name: "pubDate"; query: "pubDate/string()" }
        XmlRole { name: "category"; query: "category/string()" }
    }

    ListView {
        id: list
        anchors.fill: parent

        //anchors.top: isPortrait ? categories.bottom : window.top
        clip: true
        model: feedModel
        footer: footerText
        delegate: NewsDelegate {}
        ScrollBar.vertical: ScrollBar { }
    }

    Component {
        id: footerText

        Rectangle {
            width: parent.width
            height: childrenRect.height + 10
            color: "lightgray"

            Text {
                text: "RSS Feed from LabPlot"
                anchors.centerIn: parent
                font.pixelSize: 14
            }
        }
    }
}

