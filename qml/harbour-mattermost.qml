/*
  Copyright (C) 2017 sashikknox <sashikknox@gmail.com>

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import ru.sashikknox 1.0
import Nemo.Notifications 1.0
import "pages"
import "model"
import "cover"

ApplicationWindow
{
    id: mainwindow

    property Context context: Context {}
    property string previewString: ""

//    Notification {
//        id:notification
//        appName: "Mattermost"
//        category: "im"
//        previewBody: previewString
//    }

    Component.onCompleted: {
//        MattermostQt.newMessage.connect(
//            function onNewMessage(channel_name, user_name){
//                mainwindow.previewString =
//                        qsTr("New post on ") +
//                        channel_name +
//                        qsTr(" by user ") +
//                        user_name
//                notification.publish()
//            }
//        )
    }

    Connections {
        target: Qt.application
        onActiveChanged : {
            if( Qt.application.active ) {
//                console.debug( "Application is active")
                MattermostQt.applicationStatus = MattermostQt.AppActive
            }
            else {
//                console.debug( "Application is NOT active")
                MattermostQt.applicationStatus = MattermostQt.AppInactive
            }
        }
    }

    initialPage: Component {
        LoginPage {
            context: mainwindow.context
        }
    }

//    initialPage: Component {
//        AccountsPage {
//            context: mainwindow.context
//        }
//    }

    cover: CoverPage {
        context: mainwindow.context
    }

    allowedOrientations: defaultAllowedOrientations
}

