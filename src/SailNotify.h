#ifndef SAILNOTIFY_H
#define SAILNOTIFY_H

#include <notification.h>
#include <QObject>

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
	void slotNewMessage(QString channelname, QString user);
};

#endif // SAILNOTIFY_H
