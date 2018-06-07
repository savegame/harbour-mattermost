#ifndef SAILNOTIFY_H
#define SAILNOTIFY_H

#include <notification.h>
#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "MattermostQt.h"


struct NotificationContainer
{
	int server;
	int team;
	int channel_type;
	int channel;
	qint32 replaceId;
	Notification *notification;
};
typedef QSharedPointer<NotificationContainer> NotifyPtr;
typedef QHash<QString,NotifyPtr>  NotifyHash;
/**
 * @brief The SailNotify class
 * notification for sailfish
 */
class SailNotify : public QObject
{
	Q_OBJECT
public:
	SailNotify();

public Q_SLOTS:
	void slotNewMessage(MattermostQt::MessagePtr message);

protected:
	QHash<QString,NotifyPtr> m_notification;
};

#endif // SAILNOTIFY_H
