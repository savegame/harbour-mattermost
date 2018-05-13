#include "MessagesModel.h"

MessagesModel::MessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int MessagesModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
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
	case MessagesModel::Type:
		{
			return QVariant( (int)m_messages[index.row()]->m_type );
		}
		break;
	case MessagesModel::FilesCount:
		{
			return QVariant( (int)m_messages[index.row()]->m_file.size() );
		}
		break;
	case MessagesModel::RowIndex:
		{
			return QVariant( (int)index.row() );
		}
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
	names[MessagesModel::Type] = QLatin1String("type").data();
	names[MessagesModel::FilesCount] = QLatin1String("filescount").data();
	names[MessagesModel::RowIndex] = QLatin1String("rowindex").data();
//	names[MessagesModel::FileIcon] = QLatin1String("fileicon").data();
	return names;
}

Qt::ItemFlags MessagesModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index)
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
	connect(m_mattermost.data(), &MattermostQt::messageUpdated,
	        this, &MessagesModel::slot_messageUpdated );
}

MattermostQt *MessagesModel::getMattermost() const
{
	return m_mattermost;
}

int MessagesModel::getFileType(int row,int i) const
{
	if(row < 0 || i < 0 || row > m_messages.size() || i > m_messages[row]->m_file.size() )
		return (int)MattermostQt::FileUnknown;
	return (int)m_messages[row]->m_file[i]->m_file_type;
}

QString MessagesModel::getThumbPath(int row,int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_messages[row]->m_file[i]->m_thumb_path;
}

QSize MessagesModel::getImageSize(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return QSize();
	return m_messages[row]->m_file[i]->m_image_size;
}

QString MessagesModel::getFileName(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";
	return m_messages[row]->m_file[i]->m_name;
}

QString MessagesModel::getSenderName(int row) const
{
	if(row < 0 || row >= m_messages.size())
		return "";
	return m_messages[row]->m_user_id;
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

void MessagesModel::slot_messageUpdated(QList<MattermostQt::MessagePtr> messages)
{
	beginResetModel();
	endResetModel();
}
