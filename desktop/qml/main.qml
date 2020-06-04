import QtQuick          2.6
import QtQml.Models     2.2
import QtQuick.Controls 1.4
import ru.sashikknox    1.0
import "../qml/components"
import "../qml/panels"

Item {
    id: mainView
    width: 800
    height: 600
    readonly property real lineEditWidth: 400
    property int server_id: 0

    Connections {
        target: MattermostQt

        onServerStateChanged :{
            if( server_index !== server_id )
                console.debug("Server index is " + server_index )
//            connectionStatus = state;
            if(state === MattermostQt.ServerConnected) {
//              var teamspage = pageStack.replace(Qt.resolvedUrl("TeamsPage.qml"),
//                                                  {
//                                                      server_index: server_index,
//                                                      servername: context.mattermost.get_server_name(server_index),
//                                                      context: loginpage.context
//                                                  })
                loginPanel.buttonEnabled = false
                loginPanel.opacity = 0
                console.log("Server connected")
            }
        }

        onOnConnectionError: {
            console.log(" Error: " +message)
//            specialmessage.opacity = 1.0;
        }
    }


    LoginPanel {
        id: loginPanel
        anchors.centerIn: parent

        maxLineEditWidth: lineEditWidth

        visible: opacitu > 0
        width: Math.min(lineEditWidth + Theme.paddingLarge*4, parent.width - Theme.paddingLarge * 2)
        height: parent.height - Theme.paddingLarge * 2

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        Component.onCompleted:  {
            if ( MattermostQt.get_server_count() === 1 ) {
                server_id = 0
                serverName = MattermostQt.get_server_name(server_id)
                serverUrl  = MattermostQt.get_server_url(server_id)
    //          trust_certificate_switcher.checked = context.mattermost.get_server_trust_certificate(server_id);
            }
        }
    }
}
