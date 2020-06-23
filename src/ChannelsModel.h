#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QPointer>
#include <QSortFilterProxyModel>
#include "MattermostQt.h"

class ChannelsModel : public QAbstractListModel
{
	Q_OBJECT

	friend class ChannelsFilterProxy;

	Q_PROPERTY(MattermostQt *mattermost READ mattermost WRITE setMattermost)
	Q_PROPERTY(QString teamIcon READ teamIcon NOTIFY teamIconChanged)
public:
	enum DataRoles {
		DisplayName = Qt::DisplayRole,
		Purpose = Qt::ToolTipRole,
		Email = Qt::UserRole + 1,
		Header,
		Type,
		ChannelIndex,
		ServerIndex,
		TeamIndex,
		ChannelType,
		AvatarPath = MattermostQt::UserImageRole,
		UserStatus = MattermostQt::UserStatusRole,
		UserActivity = MattermostQt::UserLastActivityRole,
		UserName = MattermostQt::UserNameRole,
		MessageUnread,  // count of unreaded messages
		MentionCount,   // count of mention
		RoleUsersTyping, // when we have event user_typing, here a count of users
		RoleChannelId,
		RoleServerIndex,
		RoleChannelIndex,
		RoleChannelDescription,
		// -------
		RoleUserFullName, // for direct channels only
		RoleUserNickname, // for direct cahnnels only
//		Role
	};

	enum ItemType {
		HeaderFavourites = 0,
		HeaderPublic,
		HeaderPrivate,
		HeaderDirect,
		HeadersCount,
		Channel
	};
	Q_ENUMS(ItemType)

public:
	explicit ChannelsModel(QObject *parent = nullptr);

	// Header:
//	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

//	// Editable:
//	bool setData(const QModelIndex &index, const QVariant &value,
//	             int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	QHash<int, QByteArray> roleNames() const;
	// Add data:
//	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	// Remove data:
//	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	MattermostQt *mattermost();
	void setMattermost(MattermostQt *mattermost);

	QString teamIcon() const;

Q_SIGNALS:
	void teamIconChanged();

protected:
	void clear();

protected Q_SLOTS:
	void slot_channelAdded(MattermostQt::ChannelPtr channel);
	void slot_channelsList(QList<MattermostQt::ChannelPtr> list);
	void slot_updateChannel(MattermostQt::ChannelPtr channel,  QVectorInt roles);
	void slot_usersUpdated(QVector<MattermostQt::UserPtr> users,  QVectorInt roles);
	void slot_userUpdated(MattermostQt::UserPtr user,  QVectorInt roles);
	void slot_teamChanged(MattermostQt::TeamPtr team, QVectorInt roles);
private:
//	QVector<QString> m_display_name;
	QVector<QString> m_header;
	QVector<QString> m_puprose;
	QVector<MattermostQt::ChannelPtr> m_channel;
	 QVectorInt     m_type;
//	QVector<QString> m
	// indexes of headers
	 MattermostQt::TeamPtr m_team;
	int m_header_index[HeadersCount];
	QPointer<MattermostQt> m_mattermost;
};

class ChannelsFilterProxy : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	ChannelsFilterProxy(QObject *parent = Q_NULLPTR);
protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
//	virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
//	virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
private:
	void slot_sourceModelChanged();
private:
	ChannelsModel *m_model = nullptr;
};

#endif // CHANNELSMODEL_H
