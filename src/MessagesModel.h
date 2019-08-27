#ifndef MESSAGESMODEL_H
#define MESSAGESMODEL_H

#include <QAbstractListModel>

#include <QSharedPointer>
#include <QPointer>
#include <QList>
#include <QImage>
#include "MattermostQt.h"

class MessagesModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(MattermostQt *mattermost READ getMattermost WRITE setMattermost)
	Q_PROPERTY(QString channelId READ getChannelId WRITE setChannelId)
	Q_PROPERTY(bool atEnd READ atEnd NOTIFY atEndChanged)
public:
	enum DataRoles : int {
		Text = Qt::UserRole,
		Owner,
		FilesCount,
		FilePaths,
		ValidPaths,
		FileNames,
		FileStatus,
		Thumbinal,
		FileIcon,
		RowIndex,
		SenderImagePath, // user avatar
		SenderUserName,
		UserId,
		CreateAt,
		IsEdited,
		MessageIndex,
		FormatedText,
		PostId, // post id
		RootId,
		ParentId,
		OriginalId,
		RootMessage,
		RootMessageUserName,
		UserStatus = MattermostQt::UserStatusRole,
	};

public:
	explicit MessagesModel(QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	virtual QHash<int, QByteArray> roleNames() const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	void setMattermost(MattermostQt *mattermost);
	MattermostQt *getMattermost() const;

	void setChannelId(QString id);
	QString getChannelId() const;

	Q_INVOKABLE int getFileType(int row, int i) const;
	Q_INVOKABLE int getFileStatus(int row, int i) const;
	Q_INVOKABLE QString getFileMimeType(int row, int i) const;
	Q_INVOKABLE QString getThumbPath(int row, int i) const;
	Q_INVOKABLE QString getValidPath(int row, int i) const;
	Q_INVOKABLE QString getFilePath(int row, int i) const;
	Q_INVOKABLE QString getFileId(int row, int i) const;
	Q_INVOKABLE QSize   getImageSize(int row, int i) const;
	/** compute right item size for QML list view */
	Q_INVOKABLE QSizeF  getItemSize(int row, int i, qreal contentWidth) const;
	Q_INVOKABLE QString getFileName(int row, int i) const;
	Q_INVOKABLE QString getSenderName(int row) const;
	Q_INVOKABLE QString getFileSize(int row,int i) const;
	bool    atEnd() const;
//	Q_INVOKABLE int   getImageSize(int row, int i) const;

Q_SIGNALS:
	void messagesInitialized();
	void newMessage();
	void atEndChanged();
	void messagesEnded();
protected slots:
	void slot_messagesAdded(MattermostQt::ChannelPtr channel);
	void slot_messagesIsEnd(MattermostQt::ChannelPtr channel);
	void slot_messageAdded(QList<MattermostQt::MessagePtr> messages);
	void slot_messageUpdated(QList<MattermostQt::MessagePtr> messages);
	void slot_messageDeleted(QList<MattermostQt::MessagePtr> messages);
	void slot_updateMessage(MattermostQt::MessagePtr message, int role);
	void slot_messageAddedBefore(MattermostQt::ChannelPtr channel, int count);
	void slot_usersUpdated(QVector<MattermostQt::UserPtr> users, QVector<int> roles);
	void slot_userUpdated(MattermostQt::UserPtr user, QVector<int> roles);
//	void slot_fileStatusChanged(MattermostQt::FilePtr file);
private:
	QString                            m_channel_id;
	MattermostQt::ChannelPtr           m_channel;
	QVector<MattermostQt::MessagePtr>  m_messages;
//	xxQVector<MattermostQt::UserPtr>     m_users;
	QPointer<MattermostQt>             m_mattermost;
};

#endif // MESSAGESMODEL_H
