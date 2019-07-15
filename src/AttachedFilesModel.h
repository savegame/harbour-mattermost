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
	Q_PROPERTY(qreal maxWidth READ getMaxWidth WRITE setMaxWidth )
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
		FileImageSize,
		FileItemSize,
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

	qreal getMaxWidth() const ;
	void setMaxWidth(qreal value);

	Q_INVOKABLE void init(int server_index, int team_index, int channel_type, int channel_index, int message_row);

protected:
	QSizeF computeItemSize(MattermostQt::FilePtr file) const;

protected Q_SLOTS:
	void slot_attachedFilesChanged(MattermostQt::MessagePtr m, QVector<QString> file_ids, QVector<int> roles);
//	void slot_attachedFileStatusChanged(QString id, MattermostQt::FileStatus status);

protected:
	MattermostQt::ChannelPtr           m_channel;
	MattermostQt::MessagePtr           m_message;
	QPointer<MattermostQt>             m_mattermost;
	qreal                              m_maxWidth;
	bool m_init;
};

#endif // ATTACHEDFILESMODEL_H
