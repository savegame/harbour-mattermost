#include "AttachedFilesModel.h"


#define AF_DEFAULT_INT -1
#define AF_SET_DEFAULT_INT(_name) CONCAT2(m_,_name) = AF_DEFAULT_INT
#define AF_CHECK_DEF(_name) if( CONCAT2(m_,_name) == AF_DEFAULT_INT ) return;


AF_SET_PROPERTY(int, serverIndex);
AF_SET_PROPERTY(int, teamIndex);
AF_SET_PROPERTY(int, channelType);
AF_SET_PROPERTY(int, channelIndex);
AF_SET_PROPERTY(int, messageRow);

//void AttachedFilesModel:: CONCAT2(set_,channelType) ( int value )
//{
//	CONCAT2(m_,channelType) = value;
//	emit AttachedFilesModel:: AF_INDEX_SIGNAL ();
//}

AttachedFilesModel::AttachedFilesModel(QObject *parent)
{
	m_init = false;
	AF_SET_DEFAULT_INT(serverIndex);
	AF_SET_DEFAULT_INT(teamIndex);
	AF_SET_DEFAULT_INT(channelType);
	AF_SET_DEFAULT_INT(channelIndex);
	AF_SET_DEFAULT_INT(messageRow);

	connect( this, &AttachedFilesModel:: AF_INDEX_SIGNAL, this, &AttachedFilesModel::slot_onIndexChanged );
}

int AttachedFilesModel::rowCount(const QModelIndex &parent) const
{
	if(!m_message || !m_init)
		return 0;
	return m_message->m_file.size();
}

QVariant AttachedFilesModel::data(const QModelIndex &index, int role) const
{
	if(!m_message || !m_channel || !MattermostQt::getInstance() || !m_init)
		return QVariant();

	MattermostQt::FilePtr file = m_message->fileAt(index.row());
	if(file.isNull())
		return QVariant();

	if( role == FileType ) {
		return QVariant(file->m_file_type);
	}
	else if( role == FileSelfSCIndex ) {
		return file->m_self_sc_index;
	}
	else if( role == FileName ) {
		return QVariant(file->m_name);
	}
	else if( role == FilePath ) {
		return file->m_file_path;
	}
	/*
	else if( role == FileIsInCache ) {
		MattermostQt::ServerPtr s = MattermostQt::getInstance()->server()[file->m_server_index];
		return file->is_file_in_cache(s->m_cache_path);
	}
	*/
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

	else if ( file->m_file_type == MattermostQt::FileImage ||  file->m_file_type == MattermostQt::FileAnimatedImage ) {
		if( role == FileThumbnailPath ) {
			if(file->m_thumb_path.isEmpty() )
				MattermostQt::getInstance()->get_file_thumbnail(file->m_server_index, file->m_self_sc_index);
			return QVariant(file->m_thumb_path);
		}
		else if( role == FilePreviewPath ) {
			if(!file->m_preview_path.isEmpty() )
				return file->m_preview_path;
			else {
				MattermostQt::getInstance()->get_file_preview(file->m_server_index, file->m_self_sc_index);
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
	{ FileType,          "role_file_type"     },
	{ FileName,          "role_file_name"     },
	{ FileThumbnailPath, "role_thumbnail"     },
	{ FilePreviewPath,   "role_preview"       },
	{ FilePath,          "role_file_path"     },
//	{ FileIsInCache,     "role_is_in_cache"   },
	{ FileStatus,        "role_status"        },
	{ FileSize,          "role_size"          },
	{ FileMimeType,      "role_mime_type"     },
	{ FileImageSize,     "role_image_size"    },
	{ FileItemSize,      "role_item_size"     },
	{ FileId,            "role_file_id"       },
	{ FileSelfSCIndex,   "role_self_sc_index" }};
	return names;
}

void AttachedFilesModel::setMattermost(MattermostQt *mattermost)
{
	qDebug() << QStringLiteral("Set pointer to MattermostQt from QML.");
	// MattermostQt::getInstance() = mattermost;
	// MattermostQt::getInstance() = MattermostQt::getInstance()
}

MattermostQt *AttachedFilesModel::getMattermost() const
{
	return MattermostQt::getInstance();
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
	 QVectorInt roles;
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
	// qDebug() << "Init";
	if(!MattermostQt::getInstance()) {
		qCritical() << "Mattermost Client Pointer was not set!";
		return;
	}
	m_init = true;

	connect( MattermostQt::getInstance(), SIGNAL(attachedFilesChanged(MattermostQt::MessagePtr,QVector<QString>,  QVectorInt)),
	         SLOT(slot_attachedFilesChanged(MattermostQt::MessagePtr,QVector<QString>, QVectorInt)) );

	m_channel = MattermostQt::getInstance()->channelAt(server_index,team_index,channel_type,channel_index);
	if(!m_channel) {
//		qWarning() << "Cant find channel";
		return;
	}

	// becuse message row inverted from message index
	m_message = m_channel->messageAt(message_row);
	if(!m_message) {
		qCritical() << "Cant find message mi:" << message_row ;
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
			MattermostQt::getInstance()->get_file_info(server_index,team_index,channel_type,channel_index,message_row, file_id);
		}
	}
	endResetModel();
}

QSizeF AttachedFilesModel::computeItemSize(MattermostQt::FilePtr file) const
{
	// qDebug() << "computeItemSize";
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

void AttachedFilesModel::slot_attachedFilesChanged(MattermostQt::MessagePtr m, QVector<QString> file_ids,  QVectorInt roles)
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
//	QModelIndex topLeft = index(0);
//	QModelIndex bottomRight = index(m_message->m_file.size()?m_message->m_file.size()-1:0);
//	dataChanged(topLeft, bottomRight, roles);
	beginResetModel();
	endResetModel();
}

void AttachedFilesModel::slot_onIndexChanged()
{
	AF_CHECK_DEF(serverIndex);
//	AF_CHECK_DEF(teamIndex);
	if( m_teamIndex == -1 ) {
		if( channelType() != MattermostQt::ChannelDirect )
			return;
	}
	AF_CHECK_DEF(channelType);
	AF_CHECK_DEF(channelIndex);
	AF_CHECK_DEF(messageRow);
	init(m_serverIndex,m_teamIndex,m_channelType,m_channelIndex,m_messageRow);
}

//void AttachedFilesModel::slot_attachedFileStatusChanged(QString id, MattermostQt::FileStatus status)
//{
//	if( !m_message ) {
//		qCritical() << "MessagePtr is emty in AttagedFilesModel";
//		return;
//	}
//	if( m != m_message )
//		return;
//	 QVectorInt roles;
//	roles << DataRoles::FileStatus;

//	QModelIndex topLeft = index(0);
//	QModelIndex bottomRight = index(m_message->m_file.size());
//	dataChanged(topLeft, bottomRight, roles);
//}
