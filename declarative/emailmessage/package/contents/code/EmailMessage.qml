/*
*   Copyright 2010 Sebastian Kügler <sebas@kde.org>
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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

Item {
    id: mainWindow

    property string eSource: "Email-124208"

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', configChanged);
        plasmoid.busy = true
        icon.setIcon("internet-mail")
    }

    function configChanged()
    {
    }

    PlasmaCore.DataSource {
        id: emailSource
        engine: "akonadi"
        connectedSources: [eSource]
        interval: 0
        onDataChanged: {
            plasmoid.busy = false
            console.log("l0gg0r:" + connectedSources)
            //console.log("---- BODY ++++" + emailSource.data[eSource]["Body"])
        }
        Component.onCompleted: {
            console.log("Completed:" + sources)
            connectedSources = sources
        }

    }

    PlasmaCore.DataModel {
        id: dataModel
        dataSource: emailSource
        keyRoleFilter: "[\\d]*"
    }

    PlasmaCore.Theme {
        id: theme
    }

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
        property string t: emailSource.data[eSource] ? emailSource.data[eSource]["Subject"] : "empty subject"
        text: "<strong>" + t + "</strong>"
    }

    PlasmaWidgets.Label {
        id: fromLabel
        anchors.left: icon.right
        anchors.top: subjectLabel.bottom
        anchors.right: parent.right
        text: emailSource.data[eSource] ? emailSource.data[eSource]["From"] : "Unknown sender"
        opacity: .5
    }

    Flickable {
        id: flickWidget
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: fromLabel.bottom
        anchors.bottom: parent.bottom
        clip: true
        
        contentWidth: bodyView.width
        contentHeight: bodyView.height
        
        PlasmaWidgets.Label {
            id: bodyView
            width: parent.width
            height: 400
            opacity: .75
            anchors.fill: parent
            styleSheet: "text-align: top"
            property string t: emailSource.data[eSource] ? emailSource.data[eSource]["Body"] : "empty body"
            text: t
            
            Component.onCompleted: {
              console.log("email loaded.")
            }
        }               
    }
}
