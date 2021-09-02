TARGET = harbour-mattermost

# get version from yaml file
VER_VERSION=0.1.8
VER_RELEASE=1

LINES_ALL = $$cat($$PWD/rpm/harbour-mattermost.spec, lines)
for( l, LINES_ALL ): {
    contains( l, Version.* ): {
        _TMP=$$split(l," ")
        VER_VERSION=$$member(_TMP,1)
    } else : contains( l, Release:.* ) {
        _TMP=$$split(l," ")
        VER_RELEASE=$$member(_TMP,1)
    }
}

VERSION = $${VER_VERSION}-$${VER_RELEASE}
message("Version is "$$VERSION)
DEFINES += SAILFISHAPP
DEFINES += MATTERMOSTQT_VERSION=\\\"$${VERSION}\\\"
DEFINES += EMOJI_PATH=\\\"/usr/share/$$TARGET/emoji\\\"

dbus.files = dbus/sashikknox.mattermost.service
dbus.path = /usr/share/dbus-1/services/
#INSTALLS += dbus


CONFIG += sailfishapp warnoff
CONFIG += qt
QT += gui qml quick network dbus svg
PKGCONFIG += nemonotifications-qt5 sailfishsilica
#PKGCONFIG += libgcrypt

LIBS += -Llibs
INCLUDEPATH += src/

static_websocket : {
    message("Statically link with libqt5websockets")
    LIBS += -lqt5websockets
    INCLUDEPATH += libs/qtwebsockets/include
    INCLUDEPATH += libs/qtwebsockets/src/websockets
} else : {
    QT += websockets
}

emoji.files = $$PWD/resources/emoji/emoji.json
emoji.path = /usr/share/$$TARGET/emoji/
emoji_svg.files = $$PWD/resources/emoji/svg
emoji_svg.path = /usr/share/$$TARGET/emoji/
emoji_png.files = $$PWD/resources/emoji/png
emoji_png.path = /usr/share/$$TARGET/emoji/
INSTALLS += emoji emoji_svg emoji_png

debug: DEFINES += _DEBUG
!debug: DEFINES += _RELEASE
!debug: DEFINES -= _DEBUG

DISTFILES += \
    resources/emoji/* \
    rpm/harbour-mattermost.changes.run.in \
    rpm/harbour-mattermost.spec \
    translations/*.ts \
    harbour-mattermost.desktop \
    server.pri \
    translations/harbour-mattermost-ru.ts \
    dbus/sashikknox.mattermost.service \
    CHANGELOG \
    CHANGELOG \
    LICENSE \
    rpm/harbour-mattermost.changes

SAILFISHAPP_ICONS = 86x86 108x108 128x128

CONFIG += sailfishapp_i18n

include(libs/md4c.pri);

TRANSLATIONS += \
    $$PWD/translations/harbour-mattermost.ts \
    $$PWD/translations/harbour-mattermost-ru.ts \
    $$PWD/translations/harbour-mattermost-zh_CN.ts \
    $$PWD/translations/harbour-mattermost-de.ts

RESOURCES += \
    resources.qrc

HEADERS += \
    $$PWD/src/EmojiModel.h \
    $$PWD/src/MattermostQt_gobal.h \
    $$PWD/src/TeamsModel.h \
    $$PWD/src/MattermostQt.h \
    $$PWD/src/ChannelsModel.h \
    $$PWD/src/MessagesModel.h \
    $$PWD/src/SailNotify.h \
    $$PWD/src/AccountsModel.h \
    $$PWD/src/SettingsContainer.h \
    $$PWD/src/MarkdownParser.h \
    $$PWD/src/AttachedFilesModel.h \
    $$PWD/src/UsersModel.h \
    $$PWD/src/c_cpphash.h \
    $$PWD/src/cpphash.h

SOURCES += src/harbour-mattermost.cpp \
    $$PWD/src/EmojiModel.cpp \
    $$PWD/src/TeamsModel.cpp \
    $$PWD/src/MattermostQt.cpp \
    $$PWD/src/ChannelsModel.cpp \
    $$PWD/src/MessagesModel.cpp \
    $$PWD/src/SailNotify.cpp \
    $$PWD/src/AccountsModel.cpp \
    $$PWD/src/SettingsContainer.cpp \
    $$PWD/src/MarkdownParser.cpp \
    $$PWD/src/AttachedFilesModel.cpp \
    $$PWD/src/UsersModel.cpp \
    $$PWD/src/cpphash.cpp
