#ifndef SAILNOTIFY_H
#define SAILNOTIFY_H

#include <notification.h>
#include <QObject>
#include <QList>
#include "MattermostQt.h"

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
	QList<Notification*> m_notification;
};

#endif // SAILNOTIFY_H
