#include "AttachedFilesModel.h"

AttachedFilesModel::AttachedFilesModel(QObject *parent)
{

}

int AttachedFilesModel::rowCount(const QModelIndex &parent) const
{
	if(!m_message)
		return 0;
	return m_message->m_file.size();
}

QVariant AttachedFilesModel::data(const QModelIndex &index, int role) const
{
	if(!m_message)
		return QVariant();
	MattermostQt::FilePtr file = m_message->fileAt(index.row());
	if(!file)
		return QVariant();

	if( role == FileType ) {
		return QVariant(file->m_file_type);
	} else
	if( role == FileName ) {
		return QVariant(file->m_name);
	} else
	if( role == FileThumbnailPath ) {
		return QVariant(file->m_thumb_path);
	} else
	if( role == FilePreviewPath ) {
		return QVariant(file->m_preview_path);
	} else
	if( role == FilePath ) {
		return QVariant(file->m_file_path);
	} else
	if( role == FileCachePath ) {
		return QVariant();
	} else
	if( role == FileStatus ) {
		return QVariant(file->m_file_status);
	} else
	if( role == FileSize ) {
		return QVariant();
	} else
	if( role == FileMimeType ) {
		return QVariant();
	}  else
	if( role == FileId ) {
		return QVariant(file->m_id);
	}
	return QVariant();
}

QHash<int, QByteArray> AttachedFilesModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[FileType]          = QLatin1String("role_file_type" ).data();
	names[FileName]          = QLatin1String("role_file_name" ).data();
	names[FileThumbnailPath] = QLatin1String("role_thumbnail" ).data();
	names[FilePreviewPath]   = QLatin1String("role_preview"   ).data();
	names[FilePath]          = QLatin1String("role_file_path" ).data();
	names[FileCachePath]     = QLatin1String("role_cache_path").data();
	names[FileStatus]        = QLatin1String("role_status"    ).data();
	names[FileSize]          = QLatin1String("role_size"      ).data();
	names[FileMimeType]      = QLatin1String("role_mime_type" ).data();
	names[FileId]            = QLatin1String("role_file_id"   ).data();
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

void AttachedFilesModel::init(int server_index, int team_index, int channel_type, int channel_index, int message_row)
{
	if(!m_mattermost) {
		qCritical() << "Mattermost Client Pointer was not set!";
		return;
	}

	connect( m_mattermost.data(), SIGNAL(attachedFilesChanged(MattermostQt::MessagePtr,QVector<int>)),
	         SLOT(slot_attachedFilesChanged(MattermostQt::MessagePtr,QVector<int>)) );

	m_channel = m_mattermost->channelAt(server_index,team_index,channel_type,channel_index);
	if(!m_channel) {
		qWarning() << "Cant find channel";
		return;
	}
	// becuse message row inverted from message index
	int message_index = m_channel->m_message.size() - 1 - message_row;
	m_message = m_channel->messageAt(message_index);
	if(!m_message) {
		qCritical() << "Cant find message mi:" << message_index << "from mr:" << message_row;
		return;
	}

//	if(m_message->m_file_ids.size() > 0 && m_message->m_file_ids.size() != m_message->m_file.size())
	if( !m_message->m_is_files_info_requested )
	{
		m_message->m_is_files_info_requested = true;
		for( int i = 0; i < m_message->m_file_ids.size(); i++ )
			m_mattermost->get_file_info(server_index,team_index,channel_type,channel_index,message_index, m_message->m_file_ids[i]);
	}
}

void AttachedFilesModel::slot_attachedFilesChanged(MattermostQt::MessagePtr m, QVector<int> roles)
{
	if( !m_message ) {
		qCritical() << "MessagePtr is emty in AttagedFilesModel";
		return;
	}
	if( m != m_message )
		return;
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
	QModelIndex bottomRight = index(m_message->m_file.size());
	dataChanged(topLeft, bottomRight, roles);
//	beginResetModel();
//	endResetModel();
}
