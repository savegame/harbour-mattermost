TEMPLATE = subdirs
CONFIG += ordered


SUBDIRS += harbour-mattermost-app.pro

DISTFILES += \
    qml/pages/TeamOptions.qml \
    qml/model/Mattermost.qml \
    qml/pages/LoginPage.qml
