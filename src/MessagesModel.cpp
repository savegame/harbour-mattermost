#include "MessagesModel.h"

MessagesModel::MessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int MessagesModel::rowCount(const QModelIndex &parent) const
{
	return m_messages.size();
}

QVariant MessagesModel::data(const QModelIndex &index, int role) const
{
	if ( !index.isValid() || index.row() < 0 || index.row() >= m_messages.size() )
		return QVariant();

	switch (role) {
	case MessagesModel::Text:
		return QVariant(m_messages[index.row()]->m_message);
		break;
	default:
		break;
	}
	return QVariant();
}

QHash<int, QByteArray> MessagesModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[MessagesModel::Text] = QLatin1String("message").data();
	return names;
}

Qt::ItemFlags MessagesModel::flags(const QModelIndex &index) const
{
//	if (!index.isValid())
//		return Qt::NoItemFlags;
	return Qt::ItemIsEnabled; // FIXME: Implement me!
}

void MessagesModel::setMattermost(MattermostQt *mattermost)
{
	m_mattermost = mattermost;
	connect(m_mattermost.data(), &MattermostQt::messagesAdded,
	        this, &MessagesModel::slot_messagesAdded );
	connect(m_mattermost.data(), &MattermostQt::messageAdded,
	        this, &MessagesModel::slot_messageAdded );
}

void MessagesModel::slot_messagesAdded(MattermostQt::ChannelPtr channel)
{
	if(channel->m_message.size() > 0)
	{
		beginInsertRows(QModelIndex(),0,channel->m_message.size()-1);
		m_messages.append(channel->m_message);
		endInsertRows();
	}
	m_channel_id = channel->m_id;
}

void MessagesModel::slot_messageAdded(QList<MattermostQt::MessagePtr> messages)
{
	if(messages.isEmpty())
		return;
	if(messages.begin()->data()->m_channel_id.compare(m_channel_id) != 0)
		return;
	beginInsertRows(QModelIndex(),m_messages.size(),m_messages.size() + messages.size() - 1);
	foreach( MattermostQt::MessagePtr message, messages)
	{
		m_messages.append(message);
	}
	endInsertRows();
}
