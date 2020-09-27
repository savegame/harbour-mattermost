#ifndef USERSMODEL_H
#define USERSMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/qglobal.h>
#include <MattermostQt.h>

class UsersModel : public QAbstractListModel
{
	Q_OBJECT
	
	Q_PROPERTY(MattermostQt *mattermost READ mattermost WRITE setMattermost NOTIFY mattermostChanged)
public:
	explicit UsersModel(QObject *parent = nullptr);
	
	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	
	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	
	// Add data:
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	
	// Remove data:
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	
	/** Q_PROPERTY mattermost */
	MattermostQt *mattermost() const;
	void setMattermost(MattermostQt *m);
	
	/** some model API */
	Q_INVOKABLE void request_channel_members(int server_index, int team_index, int channel_type, int channel_index);
Q_SIGNALS:
	void mattermostChanged();
	
private Q_SLOTS:
	void userUpdated(MattermostQt::UserPtr user,  QVector<int> roles);
	void userAdded(MattermostQt::UserPtr user);
	
	void onReplyChannelMembers(QString channel_id, QVector<MattermostQt::UserPtr> members);
private:
	MattermostQt *m_mattermost = nullptr;
	QVector<MattermostQt::UserPtr> m_users;
};

#endif // USERSMODEL_H
