TEMPLATE = app
VERSION = 0.2.2

CONFIG += qt
QT += core gui widgets network websockets network quick #webview

TARGET = mostqute

DEFINES += DESKTOP_APP
DEFINES += MATTERMOSTQT_VERSION=\\\"$${VERSION}\\\"
#DEFINES += EMOJI_PATH=\\\"/usr/share/$$TARGET/emoji\\\"
DEFINES += EMOJI_PATH=\\\"$$PWD/resources/emoji\\\"

debug: DEFINES += _DEBUG

INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/desktop
#add discout parser
include(libs/discount.pri)
include(desktop/desktop.pri)

HEADERS += \
    src/AccountsModel.h \
    src/AttachedFilesModel.h \
    src/ChannelsModel.h \
    src/DiscountMDParser.h \
    src/EmojiModel.h \
    src/MarkdownParser.h \
    src/MattermostQt.h \
    src/MattermostQt_gobal.h \
    src/MessagesModel.h \
#    src/SailNotify.h \
    src/SettingsContainer.h \
    src/TeamsModel.h \
    src/c_cpphash.h \
    src/cpphash.h

SOURCES += \
    src/AccountsModel.cpp \
    src/AttachedFilesModel.cpp \
    src/ChannelsModel.cpp \
    src/DiscountMDParser.cpp \
    src/EmojiModel.cpp \
    src/MarkdownParser.cpp \
    src/MattermostQt.cpp \
    src/MessagesModel.cpp \
#    src/SailNotify.cpp \
    src/SettingsContainer.cpp \
    src/TeamsModel.cpp \
    src/cpphash.cpp \
#    src/harbour-mattermost.cpp



