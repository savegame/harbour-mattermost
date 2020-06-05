#include "MessagesModel.h"
#include <QDateTime>
#include <QMap>
#include <QString>
#include "SettingsContainer.h"

MessagesModel::MessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int MessagesModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	if(m_channel)
		return m_channel->m_message.size();
	return 0;
}

QVariant MessagesModel::data(const QModelIndex &index, int role) const
{
	if (!m_channel)
		return QVariant();

	if ( index.row() < 0 || index.row() >= m_channel->m_message.size() )
		return QVariant();

	int row = m_channel->m_message.size() - 1 - index.row();

	if ( row < 0 || row >= m_channel->m_message.size() )
		return QVariant();

	MattermostQt::MessagePtr message = m_channel->m_message[row];

	switch (role) {
	case MessagesModel::PostId:
		return QVariant(message->m_id);
	case MessagesModel::RootId:
		return QVariant(message->m_root_id);
	case MessagesModel::RootMessage:
		return message->m_root_message;
	case MessagesModel::RootMessageUserName:
		return message->m_root_user_name;
	case MessagesModel::ParentId:
		return QVariant(message->m_parent_id);
	case MessagesModel::OriginalId:
		return QVariant(message->m_original_id);
	case MessagesModel::Text:
		{
			bool have_unread = (m_channel->m_mention_count + m_channel->m_msg_unread > 0) && index.row() == 0;
			if( have_unread ) {
				if( m_isPageActive )
					m_mattermost->post_channel_view(
					            m_channel->m_server_index,
					            m_channel->m_team_index,
					            m_channel->m_type,
					            m_channel->m_self_index
					            );
				else
					m_request_channel_viewed = true;
			}
			return QVariant(message->m_message);
		}
		break;
	case MessagesModel::FormatedText:
		{
			bool have_unread = (m_channel->m_mention_count + m_channel->m_msg_unread > 0) && index.row() == 0;
			if( have_unread ) {
				if( m_isPageActive )
					m_mattermost->post_channel_view(
					            m_channel->m_server_index,
					            m_channel->m_team_index,
					            m_channel->m_type,
					            m_channel->m_self_index
					            );
				else
					m_request_channel_viewed = true;
			}
			if( message->m_formated_message.isEmpty() )
				message->m_formated_message = m_mattermost->parseMD(message->m_message);
			return QVariant(message->m_formated_message);
		}
		break;
	case MessagesModel::MessageIndex:
		return QVariant(message->m_self_index);
		break;
	case MessagesModel::ItemHeight:
		return QVariant(message->m_item_height);
		break;
	case MessagesModel::Owner:
		{
			return QVariant( (int)message->m_type );
		}
		break;
	case MessagesModel::FilesCount:
		{
		    return QVariant( (int)message->m_file_ids.size() );
		}
		break;
	case MessagesModel::UserId:
		{
			return QVariant( message->m_user_id );
		}
		break;
	case MessagesModel::RowIndex:
		{
			return QVariant( row );
		}
		break;
	case MessagesModel::SenderImagePath:
		{
			if( message->m_user_index >= 0
			         &&  message->m_user_index < m_mattermost->m_server[message->m_server_index]->m_user.size())
			{
				MattermostQt::UserPtr user =
				        m_mattermost->
				        m_server[message->m_server_index]->
				        m_user[message->m_user_index];
				return QVariant(user->m_image_path);
			}
			else
				return QVariant("");
		}
		break;
	case MessagesModel::SenderUserName:
		{
			if( message->m_user_index >= 0
			        &&  message->m_user_index < m_mattermost->m_server[message->m_server_index]->m_user.size())
			{
				MattermostQt::UserPtr user =
				        m_mattermost->
				        m_server[message->m_server_index]->
				        m_user[message->m_user_index];
				return QVariant(user->m_username);
			}
			else
			{
				if(m_mattermost->settings()->debug())
				{
					qWarning() << QString("Cant find USER name  message->m_user_index(%0) < size(%1) uid(%3)")
					              .arg( message->m_user_index)
					              .arg(m_mattermost->m_server[message->m_server_index]->m_user.size())
					        .arg( message->m_user_id);
				}
				return QVariant("error!");
			}
		}
		break;
	case MessagesModel::UserStatus:
		{
			if( message->m_user_index >= 0
			        &&  message->m_user_index < m_mattermost->m_server[message->m_server_index]->m_user.size())
			{
				MattermostQt::UserPtr user =
				        m_mattermost->
				        m_server[message->m_server_index]->
				        m_user[message->m_user_index];
				return QVariant(user->m_status);
			}
			else
				return QVariant(0);
		}
		break;
	case MessagesModel::ValidPaths:
		{
			if( message->m_file.size() > 0 )
			{
				QVariantList files;
				for(int i = 0; i < message->m_file.size(); i++ )
				{
					files.append( getValidPath(row,i) );
				}
				return files;
			}
			else
				return QVariantList();
		}
		break;
	case MessagesModel::FileStatus:
	    {
		if( message->m_file.size() > 0 )
		{
			QVariantList files;
			for(int i = 0; i < message->m_file.size(); i++ )
			{
				files.append( (int)message->m_file[i]->m_file_status );
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
			if( message->m_update_at  == message->m_create_at )
			{
				time = QDateTime::fromMSecsSinceEpoch(message->m_create_at);
				result = time.toString("hh:mm:ss");
			}
			else
			{
				time = QDateTime::fromMSecsSinceEpoch(message->m_update_at);
				result = time.toString("hh:mm:ss ") + QObject::tr("(edited)");
			}
			return result;
		}
		break;
	case MessagesModel::CreateDate:
		{
			QDateTime time;
			QString result;
			time = QDateTime::fromMSecsSinceEpoch(message->m_create_at);
			result = time.toString("dd.MM.yyyy"/*Qt::DefaultLocaleShortDate*/);
			return result;
		}
		break;
	case MessagesModel::IsEdited:
	    {
			return QVariant(message->m_update_at  > 0);
	    }
		break;
	case MessagesModel::RoleReactionsCount:
		return message->m_reactions.size();
	case MessagesModel::RoleReactionsPaths:
		{
			QVariantList paths;
			for( int i = 0; i < message->m_reactions.size(); i ++ )
				paths.push_back(message->m_reactions[i].m_path );
			return QVariant::fromValue(paths);
		}
	case MessagesModel::RoleReactionsAddCount:
		{
			QVariantList counts;
			for( int i = 0; i < message->m_reactions.size(); i ++ )
				counts.push_back( message->m_reactions[i].m_user_id.size() );
			return QVariant::fromValue(counts);
		}
	case MessagesModel::RoleReactionsIsMine:
		{
			QVariantList mine_reaction;
			for( int i = 0; i < message->m_reactions.size(); i ++ )
				mine_reaction.push_back( message->m_reactions[i].m_mine_emoji );
			return QVariant::fromValue(mine_reaction);
		}
	case MessagesModel::RoleReactionsEmoji:
		{
			QVariantList emoji;
			for( int i = 0; i < message->m_reactions.size(); i ++ )
				emoji.push_back( message->m_reactions[i].m_emoji );
			return QVariant::fromValue(emoji);
		}
	case MessagesModel::RoleServerIndex:
		return message->m_server_index;
	case MessagesModel::RoleTeamIndex:
		return message->m_team_index;
	case MessagesModel::RoleChannelIndex:
		return message->m_channel_index;
	case MessagesModel::RoleChannelType:
		return message->m_channel_type;
	default:
		break;
	}
	return QVariant();
}

bool MessagesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ( !m_channel || index.row() < 0 || index.row() >= m_channel->m_message.size() )
		return false;

	int row = m_channel->m_message.size() - 1 - index.row();

	if ( row < 0 || row >= m_channel->m_message.size() )
		return false;

	MattermostQt::MessagePtr message = m_channel->m_message[row];
	switch(role)
	{
	case MessagesModel::PageOrientation:
		break;
	case MessagesModel::ItemHeight:
		if(m_mattermost->settings()->debug())
			qDebug() << "Item size changed " << value.toDouble();
		message->m_item_height = value.toDouble();
		dataChanged(index,index,  QVectorInt() << role);
		break;
	default:
		return false;
	}

	return true;
}

QHash<int, QByteArray> MessagesModel::roleNames() const
{
	// thx to @Kaffeine for that optimization
	static const QHash<int, QByteArray> names = {
	{MessagesModel::Text,                  "role_message"},
	{MessagesModel::Owner,                 "role_type"},
	{MessagesModel::FilesCount,            "role_files_count"},
	{MessagesModel::RowIndex,              "role_row_index"},
	{MessagesModel::SenderImagePath,       "role_user_image_path"},
	{MessagesModel::SenderUserName,        "role_user_name"},
	{MessagesModel::CreateAt,              "role_message_create_at"},
	{MessagesModel::CreateDate,            "role_create_date"},
	{MessagesModel::IsEdited,              "role_message_is_edited"},
	{MessagesModel::UserId,                "role_user_id"},
	{MessagesModel::MessageIndex,          "role_message_index"},
	{MessagesModel::ValidPaths,            "role_valid_paths"},
	{MessagesModel::FormatedText,          "role_formated_text"},
	{MessagesModel::FileStatus,            "role_file_status"},
	{MessagesModel::UserStatus,            "role_user_status"},
	{MessagesModel::PostId,                "role_post_id"},
	{MessagesModel::RootId,                "role_root_id"},
	{MessagesModel::OriginalId,            "role_original_id"},
	{MessagesModel::ParentId,              "role_parent_id"},
	{MessagesModel::RootMessage,           "role_root_message"},
	{MessagesModel::RootMessageUserName,   "role_root_username"},
	{MessagesModel::ItemHeight,            "role_item_height"},
	{MessagesModel::PageOrientation,       "role_page_orientation"},
	{MessagesModel::RoleMessageUnread,     "role_message_unread"},
	{MessagesModel::RoleReactionsCount,    "role_reactions_count"},
	{MessagesModel::RoleReactionsPaths,    "role_reactions_paths"},
	{MessagesModel::RoleReactionsAddCount, "role_reactions_add_count"},
	{MessagesModel::RoleReactionsIsMine,   "role_reactions_is_mine"},
	{MessagesModel::RoleReactionsEmoji,    "role_reactions_emoji"},
	{MessagesModel::RoleServerIndex,       "role_server_index"},
	{MessagesModel::RoleTeamIndex,         "role_team_index"},
	{MessagesModel::RoleChannelIndex,      "role_channel_index"},
	{MessagesModel::RoleChannelType,       "role_channel_type"}};
	return names;
}

Qt::ItemFlags MessagesModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index)
//	if (!index.isValid())
//		return Qt::NoItemFlags;
	return Qt::ItemIsEnabled | Qt::ItemIsEditable; // FIXME: Implement me!
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
	connect(m_mattermost.data(), &MattermostQt::messageBeginDelete,
	        this, &MessagesModel::slot_beginMessageDeleted );
	connect(m_mattermost.data(), &MattermostQt::updateMessage,
	        this, &MessagesModel::slot_updateMessage );

	connect( m_mattermost.data(), &MattermostQt::usersUpdated,
	         this, &MessagesModel::slot_usersUpdated );
	connect( m_mattermost.data(), &MattermostQt::userUpdated,
	         this, &MessagesModel::slot_userUpdated );

	connect( m_mattermost.data(), &MattermostQt::messagesIsEnd,
	         this, &MessagesModel::slot_messagesIsEnd );

	connect( m_mattermost.data(), &MattermostQt::attachedFilesChanged,
	         this, &MessagesModel::slot_attachedFilesChanged );

	connect( m_mattermost.data(), &MattermostQt::onApplciationStatusChanged,
	         this, &MessagesModel::slot_appStatusChanged );
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
	if(m_channel)
		return m_channel->m_id;
	return QString();
}

int MessagesModel::messageCount() const
{
	if(m_channel)
		return m_channel->m_message.size();
	return 0;
}

int MessagesModel::getFileType(int row,int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return (int)MattermostQt::FileUnknown;
	return (int)m_channel->m_message[row]->m_file[i]->m_file_type;
}

int MessagesModel::getFileStatus(int row, int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return (int)MattermostQt::FileStatus::FileRemote;
	return (int)m_channel->m_message[row]->m_file[i]->m_file_status;
}

QString MessagesModel::getFileMimeType(int row, int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";
	return m_channel->m_message[row]->m_file[i]->m_mime_type;
}

QString MessagesModel::getThumbPath(int row,int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_channel->m_message[row]->m_file[i]->m_thumb_path;
}

QString MessagesModel::getValidPath(int row, int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	MattermostQt::ServerPtr sc = m_mattermost->m_server[m_channel->m_message[row]->m_server_index];
	MattermostQt::FilePtr file = m_channel->m_message[row]->m_file[i];

	if( file->m_file_type == MattermostQt::FileAnimatedImage )
	{
//		if(file->m_file_path.isEmpty())
//			return file->m_thumb_path;
		return file->m_file_path;
	}
	else if( file->m_file_size > m_mattermost->m_settings->autoDownloadImageSize() ) {
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
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_channel->m_message[row]->m_file[i]->m_file_path;
}

QString MessagesModel::getFileId(int row, int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";//TODO add path for bad thumb (default error image)
	return m_channel->m_message[row]->m_file[i]->m_id;
}

QSize MessagesModel::getImageSize(int row, int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return QSize();
	return m_channel->m_message[row]->m_file[i]->m_image_size;
}

QSizeF MessagesModel::getItemSize(int row, int i, qreal contentWidth) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return QSizeF(32,32);
	MattermostQt::FilePtr file = m_channel->m_message[row]->m_file[i];
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
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";
	return m_channel->m_message[row]->m_file[i]->m_name;
}

QString MessagesModel::getSenderName(int row) const
{
	if(!m_channel || row < 0 || row >= m_channel->m_message.size())
		return "";
	return m_channel->m_message[row]->m_user_id;
}

QString MessagesModel::getFileSize(int row, int i) const
{
	if(!m_channel || row < 0 || i < 0 || row >= m_channel->m_message.size() || i >= m_channel->m_message[row]->m_file.size() )
		return "";
	MattermostQt::FilePtr file = m_channel->m_message[row]->m_file[i];
	if( file->m_file_size < 1000 )
		return QObject::tr("%0 bytes").arg(file->m_file_size);
	qreal size = (qreal)file->m_file_size/1024;
	if( size < 1000 )
		return QObject::tr("%0 Kb").arg(size,0,'f',1);
	size = size/1024;
	return QObject::tr("%0 Mb").arg(size,0,'f',1);
}

QString MessagesModel::getReactionPath(int row, int reaction_index) const
{
	if (!m_channel)
		return QString();

//	int row = m_channel->m_message.size() - 1 - index.row();

	if ( row < 0 || row >= m_channel->m_message.size() )
		return QString();

	MattermostQt::MessagePtr message = m_channel->m_message[row];
	if( reaction_index >= message->m_reactions.size() )
		return QString();
	return message->m_reactions[reaction_index].m_path;
}

int MessagesModel::getReactionCount(int row, int reaction_index) const
{
	if (!m_channel)
		return 0;

//	int row = m_channel->m_message.size() - 1 - index.row();

	if ( row < 0 || row >= m_channel->m_message.size() )
		return 0;

	MattermostQt::MessagePtr message = m_channel->m_message[row];
	if( reaction_index >= message->m_reactions.size() )
		return 0;
	return message->m_reactions[reaction_index].m_user_id.size();
}

bool MessagesModel::atEnd() const
{
	if(!m_channel)
		return false;
	return m_channel->m_message.size() == m_channel->m_total_msg_count;
}

int MessagesModel::isPageActive() const
{
	return m_isPageActive;
}

void MessagesModel::setPageActive(bool active)
{
	if( m_isPageActive == active )
		return;
	qDebug() << QStringLiteral("Messages page status changed: active is %0").arg(active?"true":"false");
	m_isPageActive = active;
	// if status == 2 - Page is Active
	if( m_isPageActive && m_request_channel_viewed  && m_mattermost->getApplicationStatus() == MattermostQt::AppActive )
	{
		m_request_channel_viewed = false;
		m_mattermost->post_channel_view(
		        m_channel->m_server_index,
		        m_channel->m_team_index,
		        m_channel->m_type,
		        m_channel->m_self_index
		        );
	}
	emit pageStatusChanged();
}

void MessagesModel::slot_appStatusChanged()
{
	if( m_isPageActive && m_request_channel_viewed  && m_mattermost->getApplicationStatus() == MattermostQt::AppActive )
	{
		m_request_channel_viewed = false;
		m_mattermost->post_channel_view(
		        m_channel->m_server_index,
		        m_channel->m_team_index,
		        m_channel->m_type,
		        m_channel->m_self_index
		        );
	}
}


void MessagesModel::slot_messagesAdded(MattermostQt::ChannelPtr channel)
{
//	if(channel->m_message.size() != m_channel->m_message.size() )
//	{
//		beginResetModel();
//		m_channel->m_message = channel->m_message;
//		endResetModel();
//	}
#ifdef DESKTOP_APP
	m_channel = channel;
#else
	if( !m_channel )
		m_channel = channel;
	else if ( m_channel != channel) {
		return;
	}
#endif

//	beginInsertRows(QModelIndex(),0,0);
//	endInsertRows();
	beginResetModel();
	endResetModel();

	emit messagesInitialized();
	emit atEndChanged();
	emit messageCountChanged();
}

void MessagesModel::slot_messagesIsEnd(MattermostQt::ChannelPtr channel)
{
	if( m_channel == channel )
	{
		emit messagesEnded();
	}
}

void MessagesModel::slot_messageAdded(QList<MattermostQt::MessagePtr> messages)
{
	if(messages.isEmpty() )
		return;

	if(m_channel.isNull())
	{
		MattermostQt::MessagePtr m = messages.first();
		m_channel =
		        m_mattermost->channelAt(m->m_server_index, m->m_team_index, m->m_channel_type, m->m_channel_index);
		if( m_channel.isNull() ) {
			qCritical() << QStringLiteral("Cant find channel ptr");
		}
	}
	if(messages.begin()->data()->m_channel_id.compare(m_channel->m_id) != 0)
		return;
	// remeber: we has iverted messages order outside the model ;) that mean?
	// we add messages in end of array? but in bigen of model (!)
	// need refator it!
	beginInsertRows(QModelIndex(),0,messages.size() - 1);
//	m_channel->m_message.reserve();
//	foreach( MattermostQt::MessagePtr message, messages)
//	{
//		m_channel->m_message.append(message);
//	}
//	m_channel->m_message.swap(m_channel->m_message);
	endInsertRows();
	emit newMessage();
	emit messageCountChanged();
}

void MessagesModel::slot_messageUpdated(QList<MattermostQt::MessagePtr> messages)
{
//	beginResetModel();
//	endResetModel();
	if( !m_channel )
		return;

	 QVectorInt roles;
	roles << CreateAt << Text << ValidPaths << FilesCount << FormatedText << RootId << RootMessage << RootMessageUserName;
	int br = 0,lt = m_channel->m_message.size() - 1;

	QList<MattermostQt::MessagePtr> answers;
	for(QList<MattermostQt::MessagePtr>::iterator it = messages.begin(), end = messages.end(); it != end; it++ )
	{
		MattermostQt::MessagePtr m = *it;
		int row = m_channel->m_message.size() - 1 - m->m_self_index;
		QModelIndex i = index(row);
		dataChanged(i, i, roles);

		for(QList<MattermostQt::MessagePtr>::iterator
		    it_a = m->m_thread_messages.begin(), end_a = m->m_thread_messages.end();
		    it_a != end_a; it_a++ )
		{
			MattermostQt::MessagePtr ans = *it_a;
			int ans_row = m_channel->m_message.size() - 1 - ans->m_self_index;
			ans->updateRootMessage(m_mattermost.data());
			QModelIndex j = index(ans_row);
			dataChanged(j, j, roles);
		}
	}
}

void MessagesModel::slot_beginMessageDeleted(QList<MattermostQt::MessagePtr> messages)
{
	if(messages.isEmpty() || !m_channel )
		return;

	MattermostQt::MessagePtr message = messages.first();
	if(!message || message->m_channel_id.compare(m_channel->m_id) != 0)
		return;

//	int count = 0;

//	// todo тут уже должна быть модель с удаленными сообщениями
//	// т.е. либо
//	for(QList<MattermostQt::MessagePtr>::iterator
//	    it = messages.begin(), end = messages.end();
//	    it != end; it++, count++ )
//	{
//		MattermostQt::MessagePtr current = *it;
//		int row = m_channel->m_message.size() - current->m_self_index + count;
//		QModelIndex i = index(row);
//		beginRemoveRows(QModelIndex(), row, row);
//	}
	int row = m_channel->m_message.size() - 1 - message->m_self_index;
	beginRemoveRows(QModelIndex(), row, row);
}

void MessagesModel::slot_messageDeleted(QList<MattermostQt::MessagePtr> messages)
{
//	if(messages.isEmpty() || !m_channel )
//		return;

//	MattermostQt::MessagePtr message = messages.first();
//	if(!message || message->m_channel_id.compare(m_channel->m_id) != 0)
//		return;

//	int count = 0;

//	// todo тут уже должна быть модель с удаленными сообщениями
//	// т.е. либо
//	for(QList<MattermostQt::MessagePtr>::iterator
//	    it = messages.begin(), end = messages.end();
//	    it != end; it++, count++ )
//	{
//		MattermostQt::MessagePtr current = *it;
//		int row = m_channel->m_message.size() - current->m_self_index + count;
//		QModelIndex i = index(row);
//		beginRemoveRows(QModelIndex(), row, row);
////		m_channel->m_message.remove(message->m_self_index - count);
//		endRemoveRows();
//	}
	endRemoveRows();
	emit messageCountChanged();
}

void MessagesModel::slot_updateMessage(MattermostQt::MessagePtr message, int role)
{
	if( m_channel.isNull() )
		return;
	if(message->m_channel_id.compare(m_channel->m_id) != 0)
		return;
	int row = m_channel->m_message.size() - 1 - message->m_self_index;

	QVectorInt roles;
	roles << role;
	if( role == RoleReactionsCount ) {
		roles << RoleReactionsAddCount << RoleReactionsPaths << RoleReactionsIsMine;
	}
	if(m_mattermost->settings()->debug())
	{
		QHash<int, QByteArray> names = roleNames();
		if( names.contains(role) )
			qDebug() << QStringLiteral("Message %0 [%1] updated by role %2").arg(row).arg(message->m_self_index).arg(QString::fromUtf8(names[role]));
		else
			qCritical() << QStringLiteral("Undefined role %0").arg(role);
	}
	QModelIndex i = index(row);
	dataChanged(i,i,roles);

	roles.clear();
	roles << RootMessage << RootMessageUserName;
	for(QList<MattermostQt::MessagePtr>::iterator
	    it = message->m_thread_messages.begin(), end = message->m_thread_messages.end();
	    it != end; it++ )
	{
		MattermostQt::MessagePtr ans = *it;
		int ans_row = m_channel->m_message.size() - 1 - ans->m_self_index;
		ans->updateRootMessage(m_mattermost.data());
		QModelIndex i = index(ans_row);
		dataChanged(i, i, roles);
	}
}

void MessagesModel::slot_messageAddedBefore(MattermostQt::ChannelPtr channel, int count)
{
	if( channel.isNull() )
		channel = m_channel;
	else if( channel != m_channel )
		return;
//	QVector messages;
//	messages.reserve(channel->m_me
//	if(channel->m_message.size() > 0)
	{
		beginInsertRows(QModelIndex(),m_channel->m_message.size() - count - 1, m_channel->m_message.size() - 1);
//		m_channel->m_message = channel->m_message;
		endInsertRows();
	}
//	if(atEnd())
	emit atEndChanged();
	if(count < 60)
		emit messagesEnded();
	emit messageCountChanged();
}

void MessagesModel::slot_usersUpdated(QVector<MattermostQt::UserPtr> users,  QVectorInt roles)
{
	if(!m_channel)
		return;
	QModelIndex end = index(m_channel->m_message.size() - 1);
	QModelIndex begin = index(0);
	dataChanged(begin,end, roles);
}

void MessagesModel::slot_userUpdated(MattermostQt::UserPtr user,  QVectorInt roles)
{
	if( !m_channel )
		return;
	QModelIndex end = index(m_channel->m_message.size() - 1);
	QModelIndex begin = index(0);
	dataChanged(begin,end, roles);
}

void MessagesModel::slot_attachedFilesChanged(MattermostQt::MessagePtr message, QVector<QString> file_ids,  QVectorInt roles)
{
	if(!message || m_channel->m_message.empty() )
		return;
	if( message->m_channel_id != m_channel->m_message.front()->m_channel_id )
		return;

	roles.clear();
	roles << RootMessage << RootMessageUserName;

	for(QList<MattermostQt::MessagePtr>::iterator
	    it = message->m_thread_messages.begin(), end = message->m_thread_messages.end();
	    it != end; it++ )
	{
		MattermostQt::MessagePtr ans = *it;
		int ans_row = m_channel->m_message.size() - 1 - ans->m_self_index;
		ans->updateRootMessage(m_mattermost.data());
		QModelIndex i = index(ans_row);
		dataChanged(i, i, roles);
	}
}





ReverseProxyMessagesModel::ReverseProxyMessagesModel(QObject *parent)
    :QAbstractProxyModel(parent)
{

}

ReverseProxyMessagesModel::~ReverseProxyMessagesModel()
{

}

void ReverseProxyMessagesModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	QAbstractProxyModel::setSourceModel(sourceModel);
	if( MessagesModel *model = qobject_cast<MessagesModel*>(sourceModel) )
	{
		connect(model, &MessagesModel::messagesInitialized, this, &ReverseProxyMessagesModel::parentModelReseted);
		connect(model, &MessagesModel::dataChanged, this, &ReverseProxyMessagesModel::slot_dataChanged);
		connect(model, &MessagesModel::rowsAboutToBeInserted, this, &ReverseProxyMessagesModel::slot_rowsAboutToBeInserted);
		connect(model, &MessagesModel::rowsInserted, this, &ReverseProxyMessagesModel::slot_rowsInserted);
		connect(model, &MessagesModel::rowsAboutToBeRemoved, this, &ReverseProxyMessagesModel::slot_rowsAboutToBeRemoved);
		connect(model, &MessagesModel::rowsRemoved, this, &ReverseProxyMessagesModel::slot_rowsRemoved);
	}
}

QModelIndex ReverseProxyMessagesModel::mapToSource(const QModelIndex &proxyIndex) const
{
	if(!sourceModel() || proxyIndex.row() < 0 )
		return QModelIndex();
	int count = sourceModel()->rowCount();
	return createIndex(count - proxyIndex.row() - 1, 0);
}

QModelIndex ReverseProxyMessagesModel::mapFromSource(const QModelIndex &sourceIndex) const
{
	if(!sourceModel() || sourceIndex.row() < 0 )
		return QModelIndex();
	int count = sourceModel()->rowCount();
	return createIndex(count - sourceIndex.row() - 1, 0);
}

int ReverseProxyMessagesModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	if(!sourceModel())
		return 0;
	return sourceModel()->rowCount();
}

int ReverseProxyMessagesModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return 1;
}

QModelIndex ReverseProxyMessagesModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return createIndex(row,column);
}

QModelIndex ReverseProxyMessagesModel::parent(const QModelIndex &child) const
{
	Q_UNUSED(child)
	return QModelIndex();
}

void ReverseProxyMessagesModel::parentModelReseted()
{
	beginResetModel();
	endResetModel();
}

void ReverseProxyMessagesModel::slot_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
	emit dataChanged( mapFromSource(topLeft), mapFromSource(bottomRight), roles );
}

void ReverseProxyMessagesModel::slot_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
	beginInsertRows( mapFromSource(parent), sourceModel()->rowCount() - 1 - start,  sourceModel()->rowCount() - 1 - end );
}

void ReverseProxyMessagesModel::slot_rowsInserted(const QModelIndex &parent, int start, int end)
{
	endInsertRows();
}

void ReverseProxyMessagesModel::slot_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
	beginRemoveRows( mapFromSource(parent), sourceModel()->rowCount() - 1 - start,  sourceModel()->rowCount() - 1 - end );
}

void ReverseProxyMessagesModel::slot_rowsRemoved(const QModelIndex &parent, int start, int end)
{
	endRemoveRows();
}
