# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-mattermost

VERSION = 0.1.3

DEFINES += MATTERMOSTQT_VERSION=\\\"$${VERSION}\\\"

CONFIG += sailfishapp
CONFIG += qt
QT += gui qml quick network dbus svg
PKGCONFIG += nemonotifications-qt5
#PKGCONFIG += libgcrypt

LIBS += -Llibs -lqt5websockets
INCLUDEPATH += libs/qtwebsockets/include
INCLUDEPATH += libs/qtwebsockets/src/websockets
#INCLUDEPATH += libs/qtwebsockets/include
#INCLUDEPATH += libs/qtwebsockets-5.6.2/include/QtWebSockets
#INCLUDEPATH += libs/qtwebsockets-5.6.2/include

INCLUDEPATH += $$PWD/../../mersdk/targets/SailfishOS-2.1.4.13-i486/usr/include/nemonotifications-qt5

debug: DEFINES += _DEBUG
!debug: DEFINES += _RELEASE

SOURCES += src/harbour-mattermost.cpp \
    src/TeamsModel.cpp \
    src/MattermostQt.cpp \
    src/ChannelsModel.cpp \
    src/MessagesModel.cpp \
    src/SailNotify.cpp \
    src/AccountsModel.cpp \
    src/SettingsContainer.cpp


DISTFILES += \
    qml/harbour-mattermost.qml \
    qml/cover/CoverPage.qml \
    qml/pages/TeamsPage.qml \
    qml/pages/OptionsPage.qml \
    qml/components/TeamLabel.qml \
    qml/pages/AccountsPage.qml \
    qml/components/CameraPicker.qml

DISTFILES += \
    rpm/harbour-mattermost.changes.run.in \
    rpm/harbour-mattermost.spec \
    translations/*.ts \
    harbour-mattermost.desktop \
    server.pri \
    translations/harbour-mattermost-ru.ts \
    rpm/harbour-mattermost.yaml \
    dbus/sashikknox.mattermost.service \
    CHANGELOG \
    CHANGELOG \
    LICENSE \
    rpm/harbour-mattermost.changes

SAILFISHAPP_ICONS = 86x86 108x108 128x128

dbus.files = dbus/sashikknox.mattermost.service
dbus.path = /usr/share/dbus-1/services/
#INSTALLS += dbus

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-mattermost-de.ts \
    translations/harbour-mattermost-ru.ts

HEADERS += \
    src/TeamsModel.h \
    src/MattermostQt.h \
    src/ChannelsModel.h \
    src/MessagesModel.h \
    src/SailNotify.h \
    src/AccountsModel.h \
    src/SettingsContainer.h

RESOURCES += \
    resources.qrc
