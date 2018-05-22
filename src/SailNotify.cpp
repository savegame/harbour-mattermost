#include "SailNotify.h"
#include <QDebug>

SailNotify::SailNotify()
{

}

void SailNotify::slotNewMessage(QString channelname, QString user)
{
//	sNotification *n = new Notification();
	qDebug() << channelname << user;
}
