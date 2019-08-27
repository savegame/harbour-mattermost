#include "AttachedFilesModel.h"

AttachedFilesModel::AttachedFilesModel(QObject *parent)
{
	m_init = false;
}

int AttachedFilesModel::rowCount(const QModelIndex &parent) const
{
	if(!m_message || !m_init)
		return 0;
	return m_message->m_file.size();
}

QVariant AttachedFilesModel::data(const QModelIndex &index, int role) const
{
	if(!m_message || !m_channel || !m_mattermost || !m_init)
		return QVariant();

	MattermostQt::FilePtr file = m_message->fileAt(index.row());
	if(file.isNull())
		return QVariant();

	if( role == FileType ) {
		return QVariant(file->m_file_type);
	}
	else if( role == FileName ) {
		return QVariant(file->m_name);
	}
	else if( role == FilePath ) {
		return file->m_file_path;
	}
	else if( role == FileCachePath ) {
		return QVariant();
	}
	else if( role == FileStatus ) {
		return QVariant(file->m_file_status);
	}
	else if( role == FileSize ) {
		if( file->m_file_size < 1000 )
			return QObject::tr("%0 bytes").arg(file->m_file_size);
		qreal size = (qreal)file->m_file_size/1024;
		if( size < 1000 )
			return QObject::tr("%0 Kb").arg(size,0,'f',1);
		size = size/1024;
		return QObject::tr("%0 Mb").arg(size,0,'f',1);
	}
	else if( role == FileMimeType ) {
		return QVariant(file->m_mime_type);
	}
	else if( role == FileId ) {
		return QVariant(file->m_id);
	}

	else if ( file->m_file_type == MattermostQt::FileImage ) {
		if( role == FileThumbnailPath ) {
			return QVariant(file->m_thumb_path);
		}
		else if( role == FilePreviewPath ) {
			if(!file->m_preview_path.isEmpty() )
				return file->m_preview_path;
			else {
				m_mattermost->get_file_preview(file->m_server_index, file->m_self_sc_index);
				return file->m_thumb_path;
			}
		}
		else if( role == FileImageSize ) {
			return file->m_image_size;
		}
		else if ( role == FileItemSize ) {
			return computeItemSize(file);
		}
	}
	else {
		if( role == FileThumbnailPath ) {
			return QLatin1String("");
		}
		else if( role == FilePreviewPath ) {
			return QLatin1String("");
		}
		else if( role == FileImageSize ) {
			return QSizeF();
		}
		else if ( role == FileItemSize ) {
			return QSizeF();
		}
	}
	return QVariant();
}

QHash<int, QByteArray> AttachedFilesModel::roleNames() const
{
	// thx to @Kaffeine for that optimization
	static const QHash<int, QByteArray> names = {
	{ FileType,          "role_file_type"  },
	{ FileName,          "role_file_name"  },
	{ FileThumbnailPath, "role_thumbnail"  },
	{ FilePreviewPath,   "role_preview"    },
	{ FilePath,          "role_file_path"  },
	{ FileCachePath,     "role_cache_path" },
	{ FileStatus,        "role_status"     },
	{ FileSize,          "role_size"       },
	{ FileMimeType,      "role_mime_type"  },
	{ FileImageSize,     "role_image_size" },
	{ FileItemSize,      "role_item_size"  },
	{ FileId,            "role_file_id"   }};
	return names;
}

void AttachedFilesModel::setMattermost(MattermostQt *mattermost)
{
	m_mattermost = mattermost;
}

MattermostQt *AttachedFilesModel::getMattermost() const
{
	return m_mattermost.data();
}

qreal AttachedFilesModel::getMaxWidth() const
{
	return m_maxWidth;
}

void AttachedFilesModel::setMaxWidth(qreal value)
{
	m_maxWidth = value;
	if(m_message.isNull())
		return;
	QVector<int> roles;
	roles << FileItemSize;

	QModelIndex topLeft = index(0);
	QModelIndex bottomRight = index(m_message->m_file.size()?m_message->m_file.size()-1:0);
	for(int i = 0; i < m_message->m_file.size(); i++ )
	{
		m_message->m_file[i]->m_item_size = QSizeF();
	}
	dataChanged(topLeft, bottomRight, roles);
}

void AttachedFilesModel::init(int server_index, int team_index, int channel_type, int channel_index, int message_row)
{
	m_init = true;
	if(!m_mattermost) {
		qCritical() << "Mattermost Client Pointer was not set!";
		return;
	}

	connect( m_mattermost.data(), SIGNAL(attachedFilesChanged(MattermostQt::MessagePtr,QVector<QString>, QVector<int>)),
	         SLOT(slot_attachedFilesChanged(MattermostQt::MessagePtr,QVector<QString>,QVector<int>)) );

	m_channel = m_mattermost->channelAt(server_index,team_index,channel_type,channel_index);
	if(!m_channel) {
		qWarning() << "Cant find channel";
		return;
	}

	// becuse message row inverted from message index
	int message_index = message_row;
//	qDebug() << QString("This messages.size(%0)  index(%2)").arg(m_channel->m_message.size()).arg(message_index);
	m_message = m_channel->messageAt(message_index);
	if(!m_message) {
		qCritical() << "Cant find message mi:" << message_index << "from mr:" << message_row;
		return;
	}

//	qDebug() << "Files count  : " << m_message->m_file_ids.size();
	beginResetModel();
	if( m_message->m_file_ids.size() != m_message->m_file.size() )
	{
		m_message->m_is_files_info_requested = true;
		for( int i = 0; i < m_message->m_file_ids.size(); i++ )
		{
			QString file_id = m_message->m_file_ids.at(i);
			m_mattermost->get_file_info(server_index,team_index,channel_type,channel_index,message_index, file_id);
		}
	}
	endResetModel();
}

QSizeF AttachedFilesModel::computeItemSize(MattermostQt::FilePtr file) const
{
	QSize sourceSize = file->m_image_size;
	if( !file->m_item_size.isEmpty() && file->m_contentwidth == (int)m_maxWidth )
		return file->m_item_size;

	/** if file is GIF , we scae it to contentWidth */
	file->m_contentwidth = (int)m_maxWidth;
	if( sourceSize.width() > sourceSize.height() )
	{
		if( m_maxWidth > sourceSize.width() && file->m_file_type != MattermostQt::FileAnimatedImage )
		{
			file->m_item_size.setWidth( sourceSize.width() );
			file->m_item_size.setHeight( sourceSize.height() );
		}
		else
		{
			file->m_item_size.setWidth( m_maxWidth );
			file->m_item_size.setHeight( m_maxWidth/sourceSize.width() * sourceSize.height()  );
		}
	}
	else
	{
		if( m_maxWidth > sourceSize.height() && file->m_file_type != MattermostQt::FileAnimatedImage )
		{
			file->m_item_size.setWidth( sourceSize.width() );
			file->m_item_size.setHeight( sourceSize.height() );
		}
		else
		{
			file->m_item_size.setWidth( m_maxWidth/sourceSize.height() * sourceSize.width() );
			file->m_item_size.setHeight( m_maxWidth );
		}
	}
	return file->m_item_size;
}

void AttachedFilesModel::slot_attachedFilesChanged(MattermostQt::MessagePtr m, QVector<QString> file_ids, QVector<int> roles)
{
	if( !m_message ) {
		qCritical() << "MessagePtr is empty in AttachedFilesModel";
		return;
	}
	if( m != m_message )
		return;

	if( roles.isEmpty() ) // reset model (all data are updated)
	{
		beginResetModel();
		endResetModel();
		return;
	}
	for(int i = 0; i < roles.size(); i++)
	{
		if( roles[i] == FileCount )
		{
			beginResetModel();
			endResetModel();
			return;
		}
	}
	QModelIndex topLeft = index(0);
	QModelIndex bottomRight = index(m_message->m_file.size()?m_message->m_file.size()-1:0);
	dataChanged(topLeft, bottomRight, roles);
//	beginResetModel();
//	endResetModel();
}

//void AttachedFilesModel::slot_attachedFileStatusChanged(QString id, MattermostQt::FileStatus status)
//{
//	if( !m_message ) {
//		qCritical() << "MessagePtr is emty in AttagedFilesModel";
//		return;
//	}
//	if( m != m_message )
//		return;
//	QVector<int> roles;
//	roles << DataRoles::FileStatus;

//	QModelIndex topLeft = index(0);
//	QModelIndex bottomRight = index(m_message->m_file.size());
//	dataChanged(topLeft, bottomRight, roles);
//}
