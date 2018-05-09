TEMPLATE = subdirs
CONFIG += ordered


SUBDIRS += libs/qtwebsockets/src/websockets
SUBDIRS += harbour-mattermost-app.pro

DISTFILES += \
    qml/pages/MessagesPage.qml \
    qml/components/MessageLabel.qml

