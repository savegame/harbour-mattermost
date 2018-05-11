TEMPLATE = subdirs
CONFIG += ordered


SUBDIRS += libs/qtwebsockets/src/websockets
SUBDIRS += harbour-mattermost-app.pro

DISTFILES += \
    qml/components/MessageEditorBar.qml

