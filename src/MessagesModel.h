#ifndef MESSAGESMODEL_H
#define MESSAGESMODEL_H

#include <QAbstractListModel>
#include <QAbstractProxyModel>

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
	Q_PROPERTY(int count READ messageCount NOTIFY messageCountChanged)
	Q_PROPERTY(bool pageActive READ isPageActive WRITE setPageActive NOTIFY pageStatusChanged)
public:
	enum DataRoles : int {
		Text = Qt::UserRole,
		Owner,
		FilesCount,
		FilePaths,
		ValidPaths,
		FileNames,
		FileStatus,
		FilePreview,
		Thumbinal,
		FileIcon,
		RowIndex,
		SenderImagePath, // user avatar
		SenderUserName,
		UserId,
		CreateAt,
		CreateDate,
		IsEdited,
		MessageIndex,
		FormatedText,
		PostId, // post id
		RootId,
		ParentId,
		OriginalId,
		RootMessage,
		RootMessageUserName,
		ItemHeight,
		PageOrientation,
		RoleMessageUnread,
		RoleReactionsCount, // summary diferent reactions count
		RoleReactionsPaths,
		RoleReactionsAddCount, // how many users add this reaction
		RoleReactionsIsMine, // if current user too set this reaction
		RoleReactionsEmoji,
		RoleServerIndex,
		RoleTeamIndex,
		RoleChannelIndex,
		RoleChannelType,
		RoleFileInfo,
		RoleSenderFullName,
		UserStatus = MattermostQt::UserStatusRole,
	};
	Q_ENUM(DataRoles)

public:
	explicit MessagesModel(QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	virtual QHash<int, QByteArray> roleNames() const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	void setMattermost(MattermostQt *mattermost);
	MattermostQt *getMattermost() const;

	void setChannelId(QString id);
	QString getChannelId() const;

	int messageCount() const;

	Q_INVOKABLE int getFileType(int row, int i) const;
	Q_INVOKABLE MattermostQt::FilePtr getFile(int row, int i) const;
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
	Q_INVOKABLE QString getReactionPath(int row, int reaction_index) const;
	Q_INVOKABLE int getReactionCount(int row, int reaction_index) const;
	bool    atEnd() const;
	//	Q_INVOKABLE int   getImageSize(int row, int i) const;

	int isPageActive() const;
	void setPageActive(bool active);
Q_SIGNALS:
	void messagesInitialized();
	void newMessage();
	void atEndChanged();
	void messagesEnded();
	void messageCountChanged();
	void pageStatusChanged();
	void channelChanged();
protected slots:
	void slot_messagesAdded(MattermostQt::ChannelPtr channel);
	void slot_messagesIsEnd(MattermostQt::ChannelPtr channel);
	void slot_messageAdded(QList<MattermostQt::MessagePtr> messages);
	void slot_messageUpdated(QList<MattermostQt::MessagePtr> messages);
	void slot_beginMessageDeleted(QList<MattermostQt::MessagePtr> messages);
	void slot_messageDeleted(QList<MattermostQt::MessagePtr> messages);
	void slot_updateMessage(MattermostQt::MessagePtr message, int role);
	void slot_messageAddedBefore(MattermostQt::ChannelPtr channel, int count);
	void slot_usersUpdated(QVector<MattermostQt::UserPtr> users,  QVectorInt roles);
	void slot_userUpdated(MattermostQt::UserPtr user,  QVectorInt roles);
	void slot_attachedFilesChanged(MattermostQt::MessagePtr message, QVector<QString> file_ids,  QVectorInt roles);
	void slot_appStatusChanged();
private:
	QString                            m_channel_id;
	MattermostQt::ChannelPtr           m_channel;
	QPointer<MattermostQt>             m_mattermost;
	bool                               m_isPageActive = false;
	mutable bool                       m_request_channel_viewed = false;
};

class ReverseProxyMessagesModel : public QAbstractProxyModel
{
	Q_OBJECT
public:

	explicit ReverseProxyMessagesModel(QObject *parent = nullptr);
	~ReverseProxyMessagesModel();

	virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

	Q_INVOKABLE virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	Q_INVOKABLE virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

	Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	Q_INVOKABLE virtual QModelIndex index(int row, int column,
	                                      const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const override;

private Q_SLOTS:
	void parentModelReseted();
	void slot_dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> &roles);
	void slot_rowsAboutToBeInserted(const QModelIndex & parent, int start, int end);
	void slot_rowsInserted(const QModelIndex & parent, int start, int end);
	void slot_rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
	void slot_rowsRemoved(const QModelIndex & parent, int start, int end);
};

#endif // MESSAGESMODEL_H
