
// Copyright 2016 ESRI
//
// All rights reserved under the copyright laws of the United States
// and applicable international laws, treaties, and conventions.
//
// You may freely redistribute and use this sample code, with or
// without modification, provided you include the original copyright
// notice and use restrictions.
//
// See the Sample code usage restrictions document for further information.
//

import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Controls 2.2
import Esri.Cleanup 1.0
import QtGraphicalEffects 1.0
import Esri.ArcGISExtras 1.1
import Esri.ArcGISRuntime.Toolkit.Controls 100.1
import QtQuick.Dialogs 1.2
//import QtQuick.Controls.Styles 1.4
import Esri.ArcGISRuntime.Toolkit.Dialogs 100.1

Cleanup {
    id: cleanup_Columbia
    clip: true
    width: 810
    height: 600

    property double scaleFactor: System.displayScaleFactor

    // Create MapQuickView here, and create its Map etc. in C++ code
    MapView {
        id: mapView
        anchors.fill: parent
        objectName: "mapView"
        // set focus to enable keyboard navigation
        focus: true

        onMousePressed: {
            popupRect.visible = false
            resultListRect.visible = false
        }
    }

    //Map drawing
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        visible: cleanup_Columbia.mapDrawing

        RadialGradient {
            anchors.fill: parent
            opacity: 0.15
            gradient: Gradient {
                GradientStop { position: 0.0; color: "lightgrey" }
                GradientStop { position: 0.7; color: "black" }
            }
        }

        // pop up to show if MapView is drawing
        Rectangle {
            anchors.centerIn: parent
            width: 100 * scaleFactor
            height: 100 * scaleFactor
            radius: 3
            opacity: 0.85
            color: "#E0E0E0"
            border.color: "black"

            Column {
                anchors.centerIn: parent
                topPadding: 5 * scaleFactor
                spacing: 5 * scaleFactor

                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 60 * scaleFactor
                    running: true
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font {
                        weight: Font.Black
                        pixelSize: 12 * scaleFactor
                    }
                    height: 20 * scaleFactor
                    horizontalAlignment: Text.AlignHCenter
                    renderType: Text.NativeRendering
                    text: "Drawing..."
                }
            }
        }
    }

    //Legend
    Rectangle {
       id: legendRect
       anchors {
           margins: 10 * scaleFactor
           right: parent.right
           top: header.bottom
       }
       property bool expanded: true
       height: 225 * scaleFactor
       width: 250 * scaleFactor
       color: "white"
       opacity: 0.95
       radius: 5
       clip: true
       /*border {
           color: "#1eaad7"
           width: 1
       }*/

       // Animate the expand and collapse of the legend
       Behavior on height {
           SpringAnimation {
               spring: 3
               damping: .4
           }
       }

       // Catch mouse signals so they don't propagate to the map
       MouseArea {
           anchors.fill: parent
           onClicked: mouse.accepted = true
           onWheel: wheel.accepted = true
       }

       Rectangle{
           anchors {
               top: legendRect.top
               right: legendRect.right
               left: legendRect.left
           }
           height: 30 * scaleFactor
           color: "#1eaad7"
           border.color: "white"
           radius: 5
        }

       // Create UI for the user to select the layer to display
       Column {
           anchors {
               fill: parent
               margins: 7 * scaleFactor
           }
           spacing: 2 * scaleFactor

           Row {
               spacing: 170 * scaleFactor
               height: 35 * scaleFactor

               Text {
                   text: qsTr("Legend")
                   font {
                       pixelSize: 12 * scaleFactor
                       bold: true
                   }
                   color: "white"
               }

               // Legend icon to allow expanding and collapsing
               Image {
                   source: legendRect.expanded ? "qrc:/Resources/DropUp.png" : "qrc:/Resources/DropDown.png"
                   width: 22.5 * scaleFactor
                   height: 22.5 * scaleFactor

                   MouseArea {
                       anchors.fill: parent
                       onClicked: {
                           if (legendRect.expanded) {
                               legendRect.height = 30 * scaleFactor;
                               legendRect.expanded = false;
                           } else {
                               legendRect.height = 225 * scaleFactor;
                               legendRect.expanded = true;
                           }
                       }
                   }
               }
           }

           // Create a list view to display the legend
           ListView {
               id: legendListView
               anchors{
                   margins: 10 * scaleFactor
               }
               model: cleanup_Columbia.legendInfoListModel
               width: 225 * scaleFactor
               height: 175 * scaleFactor
               clip: true

               // Create delegate to display the name with an image
               delegate: Item {
                   width: parent.width
                   height: 35 * scaleFactor
                   clip: true

                   Row {
                       spacing: 5
                       anchors.verticalCenter: parent.verticalCenter
                       Image {
                           width: Math.min(symbolWidth, 24) * scaleFactor
                           height: Math.min(symbolHeight, 24) * scaleFactor
                           source: symbolUrl
                       }
                       Text {
                           width: 125 * scaleFactor
                           text: name
                           wrapMode: Text.WordWrap
                           font.pixelSize: 12 * scaleFactor
                       }

                   }
               }

               section {
                   property: "layerName"
                   criteria: ViewSection.FullString
                   labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
                   delegate: Rectangle {
                       width: 225 * scaleFactor
                       height: childrenRect.height

                       Text {
                           text: section
                           font.bold: true
                           font.pixelSize: 12 * scaleFactor
                       }
                   }
               }
           }
        }
    }

    //Popup (attributes/editing)
    Rectangle {
        id: popupRect
        anchors {
            bottomMargin: 10 * scaleFactor
            leftMargin: 10 * scaleFactor
            left: parent.left
            bottom: parent.bottom
        }
        width: parent.width/2
        height: 200 * scaleFactor
        color: "white"
        opacity: 0.95
        radius: 5
        clip: true
        border.color: "white"
        visible: false

        PopupView {
            id: puView
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                topMargin: 5 * scaleFactor
                leftMargin: 5 * scaleFactor
                rightMargin: 5 * scaleFactor
                bottomMargin: 5 * scaleFactor
            }

            popupManager: cleanup_Columbia.puManager
            backgroundColor: "transparent"
            opacity: 1
            radius: 5
            borderColor: "white"

            Connections {
                target: cleanup_Columbia
                onPuDataChanged: {
                    popupRect.visible = true
                    puView.show()
                }
            }

            Image {
                source: "qrc:/Resources/ExitButton.png"
                width: 28 * scaleFactor
                height: width
                anchors {
                    rightMargin: 10 * scaleFactor
                    topMargin: 5 * scaleFactor
                    right: parent.right
                    top: parent.top
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        cleanup_Columbia.cancelEditing();
                        popupRect.visible = false
                        puView.dismiss()
                    }
                }
            }

            Image {
                source: "qrc:/Resources/Check.png"
                width: 28 * scaleFactor
                height: width
                anchors {
                    rightMargin: 40 * scaleFactor
                    topMargin: 5 * scaleFactor
                    right: parent.right
                    top: parent.top
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        cleanup_Columbia.applyEditing();
                        popupRect.visible = false
                        puView.dismiss()
                    }
                }
            }
        }
    }

    //header
    Rectangle {
        id: header
        anchors {
            top: parent.top
            right: parent.right
            left: parent.left

            topMargin: -2 * scaleFactor
            leftMargin: -2 * scaleFactor
            rightMargin: -2 * scaleFactor
        }
        height: 50 * scaleFactor
        color: "white"
        border.color: "#1eaad7"
        border.width: 2 * scaleFactor

        MouseArea {
            anchors.fill: parent
            onClicked: mouse.accepted = true
            onWheel: wheel.accepted = true
        }

        Image {
            id: headerImg
            source: "qrc:/Resources/Cleanup_Columbia.png"
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                leftMargin: 10 * scaleFactor
                topMargin: 10 * scaleFactor
                bottomMargin: 10 * scaleFactor
            }
            width: 200 * scaleFactor
        }



        ProgressBar {
            id: progressBar
            value: cleanup_Columbia.syncProgress
            anchors {
                top: parent.top
                right: parent.right
                bottom: parent.bottom
                topMargin: 10 * scaleFactor
                rightMargin: 10 * scaleFactor
                bottomMargin: 10 * scaleFactor
            }
            height: 25 * scaleFactor
            width: 200 * scaleFactor
        }

        Text {
            text: cleanup_Columbia.syncText
            anchors {
                top: parent.top
                right: progressBar.left
                bottom: parent.bottom
                topMargin: 18 * scaleFactor
                rightMargin: 10 * scaleFactor
                bottomMargin: 10 * scaleFactor
            }
            font.bold: true
        }

        Rectangle {
            id: resultListRect
            visible: false
            anchors {
                top: findRow.bottom
                left: findRow.left
                right: findRow.right
                topMargin: 5 * scaleFactor
            }
            height: 200 * scaleFactor
            color: "transparent"

            Connections {
                target: cleanup_Columbia
                onClearResults: {
                    resultListModel.clear();
                }
                onFqResultAdded: {
                    if(result != "" && result != undefined){
                        resultListModel.append({"Result": result});
                    }
                    if(resultListModel.count >= 2){
                        resultListRect.visible = true;
                    }else{ resultListRect.visible = false; }
                }
            }

            // Catch mouse signals so they don't propagate to the map
            MouseArea {
                anchors.fill: parent
                onClicked: mouse.accepted = true
                onWheel: wheel.accepted = true
            }

            ListModel {
                id: resultListModel
                objectName: "resultListModel"
            }

            ListView {
                id: resultListView
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    topMargin: 5 * scaleFactor
                }
                height: parent.height
                model: resultListModel
                delegate: Row {
                    id: clickableResult
                    leftPadding: 10 * scaleFactor
                    topPadding: 1 * scaleFactor
                    bottomPadding: 1 * scaleFactor

                    Rectangle {
                        id: clickableResultRectangle
                        color: clickableResultMouseArea.containsMouse ? "#fad7d7" : "#ffffff"
                        width: resultListRect.width - 20 * scaleFactor
                        height: 20 * scaleFactor
                        //border.color: "#000000"
                        //border.width: 1 * scaleFactor
                        opacity: 0.95
                        radius: 5

                        Text {
                            id: clickableResultText
                            text: Result
                            anchors.fill: parent
                            anchors.leftMargin: 5 * scaleFactor
                            anchors.topMargin: 2 * scaleFactor
                        }

                        MouseArea {
                            id: clickableResultMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                resultListRect.visible = false;
                                cleanup_Columbia.runQuery(clickableResultText.text, "suggest");
                            }
                        }
                    }
                }
            }
        }

        //Querying
        Row {
            id: findRow

            anchors {
                top: parent.bottom
                left: parent.left
                leftMargin: 10 * scaleFactor
                topMargin: 10 * scaleFactor
            }
            spacing: 5

            TextField {
                id: findText

                width: 250 * scaleFactor
                height: 30 * scaleFactor
                placeholderText: "Enter something to query"
                inputMethodHints: Qt.ImhNoPredictiveText
                /*Keys.onReturnPressed: {
                    cleanup_Columbia.runQuery(findText.text, "search");
                }*/
                Keys.onReleased: {
                    cleanup_Columbia.runQuery(findText.text, "search");
                }
            }

            Button {
                id: searchButton
                height: 30 * scaleFactor

                background: Rectangle {
                    id: searchButtonRect
                    height: 30 * scaleFactor
                    radius: 5
                    border.color: searchButton.hovered ? "green" : "white"
                    color: searchButton.hovered ? "white" : "#1eaad7"

                }

                contentItem: Text {
                    topPadding: 3 * scaleFactor
                    id: searchButtonText
                    text: "Find and Select"
                    color: searchButton.hovered ? "black" : "white"
                    font.bold: true
                }

                hoverEnabled: true
                onClicked: {
                    cleanup_Columbia.runQuery(findText.text, "search");
                }
            }
        }
    }

    /*// error message dialog
    MessageDialog {
        id: errorMsgDialog
        visible: false
        text: "No results for " + findText.text.toUpperCase()
        onAccepted: {
            visible = false;
        }
    }

    onQueryFailureChanged: {
        if (cleanup_Columbia.queryFailure === true)
            errorMsgDialog.visible = true;
    }*/

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border {
            width: 0.5 * scaleFactor
            color: "black"
        }
    }

    // Declare an AuthenticationView
    AuthenticationView {
        anchors.fill: parent
        authenticationManager: cleanup_Columbia.authenticationManager // set the authenticationManager property (this needs to be registered)
    }
}
