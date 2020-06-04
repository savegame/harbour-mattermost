TEMPLATE = app
VERSION = 0.2.0

CONFIG += qt
QT += core gui widgets network websockets quick quickwidgets network

TARGET = mostqute

DEFINES += DESKTOP_APP
DEFINES += MATTERMOSTQT_VERSION=\\\"$${VERSION}\\\"
DEFINES += EMOJI_PATH=\\\"/usr/share/$$TARGET/emoji\\\"

debug: DEFINES += _DEBUG

INCLUDEPATH += $$PWD/src
#add discout parser
include(libs/discount.pri)

HEADERS += \
    desktop/AppWindow.h \
    desktop/theme.h \
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
    desktop/AppWindow.cpp \
    desktop/main.cpp \
    desktop/theme.cpp \
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

FORMS += \
    desktop/AppWindow.ui

DISTFILES += \
    desktop/qml.*

RESOURCES += \
    desktop/desktop.qrc


