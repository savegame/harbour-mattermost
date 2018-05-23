#include "SailNotify.h"
#include <QDebug>

SailNotify::SailNotify()
{

}

void SailNotify::slotNewMessage(MattermostQt::MessagePtr message)
{
	MattermostQt *m = qobject_cast<MattermostQt*>(sender());
	QString body;
	QString summary;

	QString username = m->getUserName(message->m_server_index, message->m_user_index);
	if( username.isEmpty() )
		username = message->m_user_id;

	if( message->m_channel_type == MattermostQt::ChannelDirect )
	{
		summary = username;
		body = message->m_message;
	}
	else
	{
		QString channelname = m->getChannelName(
		            message->m_server_index,
		            message->m_team_index,
		            message->m_channel_type,
		            message->m_channel_index
		            );
		if(channelname.isEmpty())
			summary = username;
		else
			summary = channelname;
		body = QString("%0: %1").arg(username).arg(message->m_message);
	}
//	sNotification *n = new Notification();
//	qDebug() << channelname << user;
	Notification notify;
//	app_name should be a string identifying the sender application, such as the name of its binary, for example. "batterynotifier"
	notify.setAppName("harbour-mattermost");
//	replaces_id should be 0 since the notification is a new one and not related to any existing notification
//	if(m_notification.empty())
//		notify.setReplacesId(0);
//	app_icon should be left empty; it will not be used in this scenario
//	summary should be left empty for nothing to be shown in the events view
//	body should be left empty for nothing to be shown in the events view
	notify.setBody(body);
	notify.setPreviewBody(body);
	notify.setSummary(summary);
	notify.setPreviewSummary(summary);
//	actions should be left empty
//	hints should contain the following:
//	category should be "device" to categorize the notification to be related to the device
//	notify.setHintValue("category",QString("device"));
	notify.setCategory("im");
//	urgency should be 2 (critical) to show the notification over the lock screen
	notify.setUrgency(Notification::Critical);
//	transient should be true to automatically close the notification after display
//	x-nemo-preview-icon should be "icon-battery-low" to define that the icon with that ID is to be shown on the preview banner
//	notify.setHintValue("x-nemo-preview-icon",QString("icon-lock-chat"));
//	x-nemo-preview-body should be "Battery low" in order to show it on the preview banner
//	notify.setHintValue("x-nemo-preview-body",QString("In %0 by %1").arg(message->m_message).arg(message->m_user_id));
//	expire_timeout should be -1 to let the notification manager choose an appropriate expiration time
//	notify.setExpireTimeout(-1);

	notify.setRemoteDBusCallServiceName("sashikknox.mattermost.service");
	notify.setRemoteDBusCallObjectPath("/sashikknox/mattermost/service");
	notify.setRemoteDBusCallInterface("sashikknox.mattermost.service");
	notify.setRemoteDBusCallMethodName("newMessage");
	QVariantList args;
	QVariant a;
	args << QVariant(message->m_server_index);
	args << QVariant(message->m_team_index);
	args << QVariant(message->m_channel_type);
	args << QVariant(message->m_channel_index);
//	args << QVariantMap( QString("server"), QVariant(message->m_server_index) );
//	args << QVariantMap( QLatin1String("team")  , QVariant(message->m_team_index)   );
//	args << QVariantMap( QLatin1String("type")  , QVariant(message->m_channel_type) );
//	args << QVariantMap( QLatin1String("chanel"), QVariant(message->m_channel_index) );

	notify.setRemoteDBusCallArguments( args );

	notify.publish();
}
