#ifndef ATTACHEDFILESMODEL_H
#define ATTACHEDFILESMODEL_H

#include <QAbstractListModel>

#include <QSharedPointer>
#include <QPointer>
#include <QList>
#include <QImage>
#include "MattermostQt.h"
#include "MattermostQt_gobal.h"

#define AF_INDEX_SIGNAL indexChanged
#define AF_PROPERTY(_type,_name) \
	MT_PROPERTY_NO_SIGNAL(_type,_name, AF_INDEX_SIGNAL)

#define AF_SET_PROPERTY(_type,_name) MT_SET_PROPERTY_SIGNAL(AttachedFilesModel, _type, _name, AF_INDEX_SIGNAL)

/** List of attached to message files */
class AttachedFilesModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(MattermostQt *mattermost READ getMattermost WRITE setMattermost)
	Q_PROPERTY(qreal maxWidth READ getMaxWidth WRITE setMaxWidth )
	AF_PROPERTY(int, serverIndex);
	AF_PROPERTY(int, teamIndex);
	AF_PROPERTY(int, channelType);
	AF_PROPERTY(int, channelIndex);
	AF_PROPERTY(int, messageRow);

	Q_SIGNAL void AF_INDEX_SIGNAL ();
public:
	enum DataRoles {
		FileType = Qt::UserRole + 100,
		FileName,
		FileThumbnailPath,
		FilePreviewPath,
		FilePath,
//		FileIsInCache,
		FileStatus,
		FileSize,
		FileMimeType,
		FileId,
		FileImageSize,
		FileItemSize,
		FileSelfSCIndex,
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
	void slot_attachedFilesChanged(MattermostQt::MessagePtr m, QVector<QString> file_ids,  QVectorInt roles);
//	void slot_attachedFileStatusChanged(QString id, MattermostQt::FileStatus status);
	void slot_onIndexChanged();
protected:
	MattermostQt::ChannelPtr           m_channel;
	MattermostQt::MessagePtr           m_message;
	QPointer<MattermostQt>             m_mattermost;
	qreal                              m_maxWidth;
	bool m_init;
};

#endif // ATTACHEDFILESMODEL_H
