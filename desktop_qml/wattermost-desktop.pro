TARGET = ru.sashikknox.wattermost

# get version from yaml file
VER_VERSION=0.1.9
VER_RELEASE=1

LINES_ALL = $$cat($$PWD/../rpm/harbour-mattermost.spec, lines)
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
DEFINES += DESKTOP_QML
DEFINES += DISABLE_SILICA_SAILFISH
DEFINES += MATTERMOSTQT_VERSION=\\\"$${VERSION}\\\"
#DEFINES += EMOJI_PATH=\\\"/usr/share/$$TARGET/emoji\\\"
DEFINES += EMOJI_PATH=\\\"$$PWD/../resources/emoji\\\"
DEFINES += _DEBUG_POST_REQUEST

dbus.files = dbus/sashikknox.mattermost.service
dbus.path = /usr/share/dbus-1/services/
#INSTALLS += dbus

CONFIG += warnoff
CONFIG += qt

QT += gui qml quick network dbus svg quickwidgets widgets
#PKGCONFIG += libgcrypt

LIBS += -Llibs
INCLUDEPATH += $$PWD/../src/
INCLUDEPATH += $$PWD/..

QT += websockets

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
    ../resources/emoji/* \
    qml/components/BaseButton.qml \
    qml/components/CButton.qml \
    qml/components/CCheckBox.qml \
    qml/components/CLabel.qml \
    qml/components/IconButton.qml \
    qml/components/LineEdit.qml \
    qml/components/PageHeader.qml \
    qml/components/Panel.qml \
    qml/components/ScrollBar.qml \
    qml/items/AvatarItem.qml \
    qml/items/ImageFileItem.qml \
    qml/items/MessageEditorItem.qml \
    qml/items/MessageLabelItem.qml \
    qml/main.qml \
    qml/panels/ChannelsPanel.qml \
    qml/panels/LoginPanel.qml \
    qml/panels/MessagesPanel.qml \
    qml/panels/TeamsPanel.qml

include(../libs/md4c.pri);

TRANSLATIONS += \
    $$PWD/translations/wattermost.ts \
    $$PWD/translations/wattermost-ru.ts

RESOURCES += \
    desktop.qrc

HEADERS += \
    $$PWD/../src/EmojiModel.h \
    $$PWD/../src/MattermostQt_gobal.h \
    $$PWD/../src/TeamsModel.h \
    $$PWD/../src/MattermostQt.h \
    $$PWD/../src/ChannelsModel.h \
    $$PWD/../src/MessagesModel.h \
#    $$PWD/../src/SailNotify.h \
    $$PWD/../src/AccountsModel.h \
    $$PWD/../src/SettingsContainer.h \
    $$PWD/../src/MarkdownParser.h \
    $$PWD/../src/AttachedFilesModel.h \
    $$PWD/../src/UsersModel.h \
    $$PWD/../src/c_cpphash.h \
    $$PWD/../src/cpphash.h \
    $$PWD/AppWindow.h \
    $$PWD/theme.h

SOURCES += main.cpp \
    $$PWD/../src/EmojiModel.cpp \
    $$PWD/../src/TeamsModel.cpp \
    $$PWD/../src/MattermostQt.cpp \
    $$PWD/../src/ChannelsModel.cpp \
    $$PWD/../src/MessagesModel.cpp \
#    $$PWD/../src/SailNotify.cpp \
    $$PWD/../src/AccountsModel.cpp \
    $$PWD/../src/SettingsContainer.cpp \
    $$PWD/../src/MarkdownParser.cpp \
    $$PWD/../src/AttachedFilesModel.cpp \
    $$PWD/../src/UsersModel.cpp \
    $$PWD/../src/cpphash.cpp \
    $$PWD/AppWindow.cpp \
    $$PWD/theme.cpp

FORMS += \
    $$PWD/AppWindow.ui
