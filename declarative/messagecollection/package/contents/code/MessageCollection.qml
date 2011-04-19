/*
*   Copyright 2011 Sebastian Kügler <sebas@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: mainWindow
    //width: 300
    //height: 400

    property string collection: "EmailCollection-109"
    property string source
    property variant individualSources
    property int scrollInterval

    Component.onCompleted: {
        //plasmoid.addEventListener('ConfigChanged', configChanged);
        plasmoid.busy = true
        //icon.setIcon("internet-mail")
    }

    PlasmaCore.DataSource {
        id: collectionSource
        engine: "akonadi"
        connectedSources: [collection]
        interval: 50000
        onDataChanged: {
            //console.log("datachanged" + source)
            plasmoid.busy = false
        }
        onSourceAdded: {
            //var s = new QString(source);

            if (source.match("Email-")) {
                console.log("connecting: " + source)
                connectSource(source)
            } else {
                console.log("ignoring: " + source)
            }
        }
        Component.onCompleted: {
            console.log("Completed:" + sources)
            connectedSources = sources
        }
    }

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaWidgets.Label {
        id: title
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18nc("title", "<h2>Messages</h2>")
    }

    ListView {
        id: entryList
        spacing: 10;
        snapMode: ListView.SnapToItem
        orientation: ListView.Vertical
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        highlightMoveDuration: 300
        property int listIndex: index

        model: PlasmaCore.DataModel {
            dataSource: collectionSource
            //keyRoleFilter: "Email-[\\d]"
        }

        delegate: Item {
            property bool expanded: false
            property int collapsedHeight: 38
            property int expandedHeight: 200

            id: emailItem
            width: entryList.width
            height: collapsedHeight

            PlasmaWidgets.IconWidget {
                id: icon
                width: 48
                height: 48
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: flickWidget.top
                onClicked: {
                    console.log("click");
                }
            }

            PlasmaWidgets.Label {
                id: subjectLabel
                anchors.left: icon.right
                anchors.top: parent.top
                anchors.right: parent.right
                property string t: Subject
                //emailSource.data[eSource] ? emailSource.data[eSource]["Subject"] : "empty subject"
                text: "<strong>" + t + "</strong>"
                //text: subject
            }

            PlasmaWidgets.Label {
                id: fromLabel
                anchors.left: icon.right
                anchors.top: subjectLabel.bottom
                anchors.right: parent.right
                //text: emailSource.data[eSource] ? emailSource.data[eSource]["From"] : "Unknown sender"
                //text: "from " + collectionSource.data["emailMessage-121771"]["from"] + ", received today"
                //text: "From: someone <some@one.org>"// + From
                text: "From: " + From
                opacity: .5
            }

            Flickable {
                id: flickWidget
                anchors.left: icon.right
                anchors.right: fromLabel.right
                anchors.top: fromLabel.bottom
                anchors.bottom: parent.bottom
                clip: true

                contentWidth: bodyView.width
                contentHeight: bodyView.height

                PlasmaWidgets.Label {
                    id: bodyView
                    anchors.fill: parent
                    opacity: .75
                    styleSheet: "text-align: top"
                    //property string t: emailSource.data[eSource] ? emailSource.data[eSource]["Body"] : "empty body"
                    //text: model.dataSource["body"]
                    //text: "this is the real message. So say hi! \n\n\n HI!\n\n-- sebas"
                    text: Body
                }
            }

            PropertyAnimation {
                id: growAnimation;
                target: emailItem;
                property: "height";
                to: expandedHeight;
                duration: 300;
                easing.type: Easing.InOutElastic;
                easing.amplitude: 2.0; easing.period: 1.5
            }

            PropertyAnimation {
                id: shrinkAnimation;
                target: emailItem;
                property: "height";
                to: collapsedHeight
                duration: 300;
                easing.type: Easing.InOutElastic;
                easing.amplitude: 2.0; easing.period: 1.5
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    print("clicked: " + subjectLabel.text)
                    if (!expanded) {
                        growAnimation.running = true
                    } else {
                        shrinkAnimation.running = true
                    }
                    expanded = !expanded
                    //plasmoid.openUrl(Url)
                }
            }

            Component.onCompleted: {
                icon.setIcon("internet-mail")
            }
        }

        ListView.onAdd: {
            console.log("added...")
            plasmoid.busy = false
        }

        onFlickEnded: {
            currentIndex = contentX / contentWidth * count
        }
        Timer {
            id: flickTimer
            interval: scrollInterval * 1000
            running: true
            repeat: true
            onTriggered: {
                if (entryList.currentIndex == (entryList.count - 1))
                    entryList.currentIndex = 0
                else
                    entryList.currentIndex = entryList.currentIndex + 1
            }
        }
    }
}
