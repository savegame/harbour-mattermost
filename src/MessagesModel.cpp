#include "MessagesModel.h"
#include <QDateTime>

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

//	if(index.row() == 0 && m_channel->m_message.size() < m_channel->m_total_msg_count && m_channel->m_message.size() > 0)
//		m_mattermost->get_posts_before(
//		            m_channel->m_server_index,
//		            m_channel->m_team_index,
//		            m_channel->m_self_index,
//		            m_channel->m_type
//		            );
	int row = m_messages.size() - 1 - index.row();
	switch (role) {
	case MessagesModel::Text:
		return QVariant(m_messages[row]->m_message);
		break;
	case MessagesModel::MessageIndex:
		return QVariant(m_messages[row]->m_self_index);
		break;
	case MessagesModel::Type:
		{
			return QVariant( (int)m_messages[row]->m_type );
		}
		break;
	case MessagesModel::FilesCount:
		{
			return QVariant( (int)m_messages[row]->m_file.size() );
		}
		break;
	case MessagesModel::UserId:
		{
			return QVariant( m_messages[row]->m_user_id );
		}
		break;
	case MessagesModel::RowIndex:
		{
			return QVariant( row );
		}
		break;
	case MessagesModel::SenderImagePath:
		{
			if( m_messages[row]->m_user_index >= 0
			         &&  m_messages[row]->m_user_index < m_mattermost->m_server[m_messages[row]->m_server_index]->m_user.size())
			{
				MattermostQt::UserPtr user =
				        m_mattermost->
				        m_server[m_messages[row]->m_server_index]->
				        m_user[m_messages[row]->m_user_index];
				return QVariant(user->m_image_path);
			}
			else
				return QVariant("");
		}
		break;
	case MessagesModel::SenderUserName:
		{
			if( m_messages[row]->m_user_index >= 0
			        &&  m_messages[row]->m_user_index < m_mattermost->m_server[m_messages[row]->m_server_index]->m_user.size())
			{
				MattermostQt::UserPtr user =
				        m_mattermost->
				        m_server[m_messages[row]->m_server_index]->
				        m_user[m_messages[row]->m_user_index];
				return QVariant(user->m_username);
			}
			else
				return QVariant("");
		}
		break;
	case MessagesModel::ValidPaths:
		{
			if( m_messages[row]->m_file.size() > 0 )
			{
				QVariantList files;
				for(int i = 0; i < m_messages[row]->m_file.size(); i++ )
				{
					files.append( getValidPath(row,i) );
				}
				return files;
			}
			else
				return QVariantList();
		}
		break;
	case MessagesModel::CreateAt:
		{
			QDateTime time;
			QString result;
			if( m_messages[row]->m_update_at  == m_messages[row]->m_create_at )
			{
				time = QDateTime::fromMSecsSinceEpoch(m_messages[row]->m_create_at);
				result = time.toString("hh:mm:ss");
			}
			else
			{
				time = QDateTime::fromMSecsSinceEpoch(m_messages[row]->m_update_at);
				result = time.toString("hh:mm:ss ") + QObject::trUtf8("(edited)");
			}
			return result;
		}
		break;
	case MessagesModel::IsEdited:
	    {
		    return QVariant(m_messages[row]->m_update_at  > 0);
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
	names[MessagesModel::SenderImagePath] = QLatin1String("userimagepath").data();
	names[MessagesModel::SenderUserName] = QLatin1String("user").data();
	names[MessagesModel::CreateAt] = QLatin1String("messagecreateat").data();
	names[MessagesModel::IsEdited] = QLatin1String("messageisedited").data();
	names[MessagesModel::UserId] = QLatin1String("userid").data();
	names[MessagesModel::MessageIndex] = QLatin1String("messageindex").data();
//	names[MessagesModel::FilePaths] = QLatin1String("filepaths").data();
	names[MessagesModel::ValidPaths] = QLatin1String("validpaths").data();
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
	connect(m_mattermost.data(), &MattermostQt::messagesAddedBefore,
	        this, &MessagesModel::slot_messageAddedBefore );
	connect(m_mattermost.data(), &MattermostQt::messageUpdated,
	        this, &MessagesModel::slot_messageUpdated );
	connect(m_mattermost.data(), &MattermostQt::messageDeleted,
	        this, &MessagesModel::slot_messageDeleted );
	connect(m_mattermost.data(), &MattermostQt::updateMessage,
	        this, &MessagesModel::slot_updateMessage );
}

MattermostQt *MessagesModel::getMattermost() const
{
	return m_mattermost;
}

void MessagesModel::setChannelId(QString id)
{
	m_channel_id = id;
}

QString MessagesModel::getChannelId() const
{
	return m_channel_id;
}

int MessagesModel::getFileType(int row,int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return (int)MattermostQt::FileUnknown;
	return (int)m_messages[row]->m_file[i]->m_file_type;
}

int MessagesModel::getFileStatus(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return (int)MattermostQt::FileStatus::FileRemote;
	return (int)m_messages[row]->m_file[i]->m_file_status;
}

QString MessagesModel::getFileMimeType(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";
	return m_messages[row]->m_file[i]->m_mime_type;
}

QString MessagesModel::getThumbPath(int row,int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_messages[row]->m_file[i]->m_thumb_path;
}

QString MessagesModel::getValidPath(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	MattermostQt::ServerPtr sc = m_mattermost->m_server[m_messages[row]->m_server_index];
	MattermostQt::FilePtr file = m_messages[row]->m_file[i];

	if( file->m_file_type == MattermostQt::FileAnimatedImage )
	{
//		if(file->m_file_path.isEmpty())
//			return file->m_thumb_path;
		return file->m_file_path;
	}
	else if( file->m_file_size > m_mattermost->m_settings->m_auto_download_image_size) {
		if(!file->m_preview_path.isEmpty() )
			return file->m_preview_path;
		else {
			m_mattermost->get_file_preview(file->m_server_index, file->m_self_sc_index);
			return file->m_thumb_path;
		}
	}
	return file->m_file_path;
}

QString MessagesModel::getFilePath(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_messages[row]->m_file[i]->m_file_path;
}

QString MessagesModel::getFileId(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_messages[row]->m_file[i]->m_id;
}

QSize MessagesModel::getImageSize(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return QSize();
	return m_messages[row]->m_file[i]->m_image_size;
}

QSizeF MessagesModel::getItemSize(int row, int i, qreal contentWidth) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return QSizeF();
	MattermostQt::FilePtr file = m_messages[row]->m_file[i];
	QSize sourceSize = file->m_image_size;
	if( !file->m_item_size.isEmpty() && file->m_contentwidth == (int)contentWidth )
		return file->m_item_size;

	/** if file is GIF , we scae it to contentWidth */
	file->m_contentwidth = (int)contentWidth;
	if( sourceSize.width() > sourceSize.height() )
	{
		if( contentWidth > sourceSize.width() && file->m_file_type != MattermostQt::FileAnimatedImage )
		{
			file->m_item_size.setWidth( sourceSize.width() );
			file->m_item_size.setHeight( sourceSize.height() );
		}
		else
		{
			file->m_item_size.setWidth( contentWidth );
			file->m_item_size.setHeight( contentWidth/sourceSize.width() * sourceSize.height()  );
		}
	}
	else
	{
		if( contentWidth > sourceSize.height() && file->m_file_type != MattermostQt::FileAnimatedImage )
		{
			file->m_item_size.setWidth( sourceSize.width() );
			file->m_item_size.setHeight( sourceSize.height() );
		}
		else
		{
			file->m_item_size.setWidth( contentWidth/sourceSize.height() * sourceSize.width() );
			file->m_item_size.setHeight( contentWidth );
		}
	}
	return file->m_item_size;
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

QString MessagesModel::getFileSize(int row, int i) const
{
	if(row < 0 || i < 0 || row >= m_messages.size() || i >= m_messages[row]->m_file.size() )
		return "";
	MattermostQt::FilePtr file = m_messages[row]->m_file[i];
	if( file->m_file_size < 1000 )
		return QObject::trUtf8("%0 bytes").arg(file->m_file_size);
	qreal size = (qreal)file->m_file_size/1024;
	if( size < 1000 )
		return QObject::trUtf8("%0 Kb").arg(size,0,'f',1);
	size = size/1024;
	return QObject::trUtf8("%0 Mb").arg(size,0,'f',1);
}

bool MessagesModel::atEnd() const
{
	if(!m_channel)
		return false;
	return m_channel->m_message.size() == m_channel->m_total_msg_count;
}

void MessagesModel::slot_messagesAdded(MattermostQt::ChannelPtr channel)
{
	if(channel->m_message.size() != m_messages.size() )
	{
//		m_messages.reserve(channel->m_message.size());
//		beginInsertRows(QModelIndex(),0,channel->m_message.size()-1);
//		m_messages.append(channel->m_message);
		beginResetModel();
		m_messages = channel->m_message;
//		endInsertRows();
		endResetModel();
	}
	m_channel = channel;
//	if(atEnd())
	emit messagesInitialized();
	emit atEndChanged();
}

void MessagesModel::slot_messageAdded(QList<MattermostQt::MessagePtr> messages)
{
	if(messages.isEmpty())
		return;
	if(messages.begin()->data()->m_channel_id.compare(m_channel->m_id) != 0)
		return;
	// remeber: we has iverted messages order outside the model ;) that mean?
	// we add messages in end of array? but in bigen of model (!)
	// need refator it!
	beginInsertRows(QModelIndex(),0,messages.size() - 1);
//	m_messages.reserve();
	foreach( MattermostQt::MessagePtr message, messages)
	{
		m_messages.append(message);
	}
//	m_messages.swap(m_channel->m_message);
	endInsertRows();
	emit newMessage();
}

void MessagesModel::slot_messageUpdated(QList<MattermostQt::MessagePtr> messages)
{
//	beginResetModel();
//	endResetModel();

	QVector<int> roles;
	roles << CreateAt << Text << ValidPaths << FilesCount;
	int br = 0,lt = m_messages.size() - 1;
	foreach(MattermostQt::MessagePtr m, messages)
	{
		lt = (m->m_self_index < lt)?m->m_self_index:lt;
		br = (m->m_self_index > br)?m->m_self_index:br;
	}
	lt = m_messages.size() - 1 - lt;
	br = m_messages.size() - 1 - br;
	QModelIndex topLeft = index(br);
	QModelIndex bottomRight = index(lt);
	dataChanged(topLeft, bottomRight, roles);
}

void MessagesModel::slot_messageDeleted(MattermostQt::MessagePtr message)
{
	if(message->m_channel_id.compare(m_channel->m_id) != 0)
		return;

	int row = m_messages.size() - 1 - message->m_self_index;
	beginRemoveRows(QModelIndex(), row, row);
	m_messages.remove(message->m_self_index);
	endRemoveRows();

//	QVector<int> roles;
//	roles << MessageIndex;
//	int br = m_messages.size() - 1;
//	QModelIndex topLeft = index(0);
//	QModelIndex bottomRight = index(m_messages.size() - 1);
	//	dataChanged(topLeft, bottomRight, roles);
}

void MessagesModel::slot_updateMessage(MattermostQt::MessagePtr message, int role)
{
	if(message->m_channel_id.compare(m_channel->m_id) != 0)
		return;
	int row = m_messages.size() - 1 - message->m_self_index;

	QVector<int> roles;
	roles << role;
	QModelIndex i = index(row);
	dataChanged(i,i,roles);
}

void MessagesModel::slot_messageAddedBefore(MattermostQt::ChannelPtr channel, int count)
{
	if( channel != m_channel )
		return;
//	QVector messages;
//	messages.reserve(channel->m_me
//	if(channel->m_message.size() > 0)
	{
		beginInsertRows(QModelIndex(),m_messages.size(),m_messages.size() + count-1);
		m_messages = channel->m_message;
		endInsertRows();
	}
//	if(atEnd())
	emit atEndChanged();
}



