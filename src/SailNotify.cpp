#include "SailNotify.h"
#include <QDebug>

SailNotify::SailNotify()
{

}

void SailNotify::slotNewMessage(QString channelname, QString user)
{
//	sNotification *n = new Notification();
//	qDebug() << channelname << user;
	Notification *notify = new Notification(this);
//	app_name should be a string identifying the sender application, such as the name of its binary, for example. "batterynotifier"
	notify->setAppName("harbour-mattermost");
//	replaces_id should be 0 since the notification is a new one and not related to any existing notification
	if(m_notification.empty())
		notify->setReplacesId(0);
//	app_icon should be left empty; it will not be used in this scenario
//	summary should be left empty for nothing to be shown in the events view
	notify->setSummary(channelname);
//	body should be left empty for nothing to be shown in the events view
	notify->setBody(user);
//	actions should be left empty
//	hints should contain the following:
//	category should be "device" to categorize the notification to be related to the device
	notify->setHintValue("category",QString("device"));
//	urgency should be 2 (critical) to show the notification over the lock screen
	notify->setHintValue("urgency",QVariant(2) );
//	transient should be true to automatically close the notification after display
//	x-nemo-preview-icon should be "icon-battery-low" to define that the icon with that ID is to be shown on the preview banner
	notify->setHintValue("x-nemo-preview-icon",QString("icon-lock-chat"));
//	x-nemo-preview-body should be "Battery low" in order to show it on the preview banner
	notify->setHintValue("x-nemo-preview-body",QString("In %0 by %1").arg(channelname).arg(user));
//	expire_timeout should be -1 to let the notification manager choose an appropriate expiration time
	notify->setExpireTimeout(-1);

	notify->publish();
}
