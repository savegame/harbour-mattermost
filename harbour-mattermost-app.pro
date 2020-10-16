TARGET = harbour-mattermost

# get version from yaml file
VER_VERSION=0.1.7
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

include(libs/discount.pri);
include(libs/md4c.pri);

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

SOURCES += src/harbour-mattermost.cpp \
    src/EmojiModel.cpp \
    src/TeamsModel.cpp \
    src/MattermostQt.cpp \
    src/ChannelsModel.cpp \
    src/MessagesModel.cpp \
    src/SailNotify.cpp \
    src/AccountsModel.cpp \
    src/SettingsContainer.cpp \
    src/MarkdownParser.cpp \
    src/DiscountMDParser.cpp \
    src/AttachedFilesModel.cpp \
    src/UsersModel.cpp \
    src/cpphash.cpp

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

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n


# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += \
    translations/harbour-mattermost.ts \
    translations/harbour-mattermost-ru.ts \
    translations/harbour-mattermost-zh_CN.ts \
    translations/harbour-mattermost-de.ts

HEADERS += \
    src/EmojiModel.h \
    src/MattermostQt_gobal.h \
    src/TeamsModel.h \
    src/MattermostQt.h \
    src/ChannelsModel.h \
    src/MessagesModel.h \
    src/SailNotify.h \
    src/AccountsModel.h \
    src/SettingsContainer.h \
    src/MarkdownParser.h \
    src/DiscountMDParser.h \
    src/AttachedFilesModel.h \
    src/UsersModel.h \
    src/c_cpphash.h \
    src/cpphash.h

RESOURCES += \
    resources.qrc
