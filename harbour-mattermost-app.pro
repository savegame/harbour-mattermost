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

VERSION = 0.1.0

DEFINES += MATTERMOSTQT_VERSION=\\\"$${VERSION}\\\"

CONFIG += sailfishapp
CONFIG += qt
QT += gui qml quick network dbus
PKGCONFIG += \
    nemonotifications-qt5

LIBS += -Llibs -lqt5websockets
INCLUDEPATH += libs/qtwebsockets/include/QtWebSockets

INCLUDEPATH += $$PWD/../../mersdk/targets/SailfishOS-2.1.4.13-i486/usr/include/nemonotifications-qt5

debug: DEFINES += _DEBUG

SOURCES += src/harbour-mattermost.cpp \
    src/TeamsModel.cpp \
    src/MattermostQt.cpp \
    src/ChannelsModel.cpp \
    src/MessagesModel.cpp \
    src/SailNotify.cpp

DISTFILES += qml/harbour-mattermost.qml \
    qml/cover/CoverPage.qml \
    rpm/harbour-mattermost.changes.in \
    rpm/harbour-mattermost.changes.run.in \
    rpm/harbour-mattermost.spec \
    translations/*.ts \
    harbour-mattermost.desktop \
    server.pri \
    qml/pages/TeamsPage.qml \
    qml/components/TeamLabel.qml \
    translations/harbour-mattermost-ru.ts \
    rpm/harbour-mattermost.yaml \
    dbus/sashikknox.mattermost.service \
    qml/pages/OptionsPage.qml

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
    src/SailNotify.h

RESOURCES += \
    resources.qrc
