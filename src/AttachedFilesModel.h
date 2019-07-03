#ifndef ATTACHEDFILESMODEL_H
#define ATTACHEDFILESMODEL_H

#include <QAbstractListModel>

#include <QSharedPointer>
#include <QPointer>
#include <QList>
#include <QImage>
#include "MattermostQt.h"

/** List of attached to message files */
class AttachedFilesModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(MattermostQt *mattermost READ getMattermost WRITE setMattermost)

public:
	enum DataRoles {
		FileType = 0,
		FileName,
		FileThumbnailPath,
		FilePreviewPath,
		FilePath,
		FileCachePath,
		FileStatus,
		FileSize,
		FileMimeType,
		FileId,
		FileCount,
	};
	Q_ENUM(DataRoles)
public:
	AttachedFilesModel(QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	virtual QHash<int, QByteArray> roleNames() const override;

	void setMattermost(MattermostQt *mattermost);
	MattermostQt *getMattermost() const;

	Q_INVOKABLE void init(int server_index, int team_index, int channel_type, int channel_index, int message_row);

protected Q_SLOTS:
	void slot_attachedFilesChanged(MattermostQt::MessagePtr m, QVector<int> roles);

protected:
	MattermostQt::ChannelPtr           m_channel;
	MattermostQt::MessagePtr           m_message;
	QPointer<MattermostQt>             m_mattermost;
	bool m_init;
};

#endif // ATTACHEDFILESMODEL_H
