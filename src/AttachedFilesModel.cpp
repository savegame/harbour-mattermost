#include "AttachedFilesModel.h"

AttachedFilesModel::AttachedFilesModel(QObject *parent)
{

}

int AttachedFilesModel::rowCount(const QModelIndex &parent) const
{

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
	}
	return QVariant();
}

QHash<int, QByteArray> AttachedFilesModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[FileType]          = QLatin1String("role_file_type" );
	names[FileName]          = QLatin1String("role_file_name" );
	names[FileThumbnailPath] = QLatin1String("role_thumbnail" );
	names[FilePreviewPath]   = QLatin1String("role_preview"   );
	names[FilePath]          = QLatin1String("role_file_path" );
	names[FileCachePath]     = QLatin1String("role_cache_path");
	names[FileStatus]        = QLatin1String("role_status"    );
	names[FileSize]          = QLatin1String("role_size"      );
	names[FileMimeType]      = QLatin1String("role_mime_type" );
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

void AttachedFilesModel::init(int server_index, int team_index, int channel_type, int channel_index, int message_index)
{
	if(!m_mattermost) {
		qCritical() << "Mattermost Client Pointer not set!";
		return;
	}
	m_channel = m_mattermost->channelAt(server_index,team_index,channel_type,channel_index);
	if(!m_channel) {
		qWarning() << "Cant find channel";
		return;
	}
	m_message = m_channel->messageAt(message_index);
	if(!m_message) {
		qCritical() << "Cant find message";
		return;
	}
}
