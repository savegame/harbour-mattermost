#ifndef USERSMODEL_H
#define USERSMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/qglobal.h>
#include <MattermostQt.h>

class UsersModelPrivate;
class UsersModel : public QAbstractListModel
{
	Q_OBJECT
	
	Q_PROPERTY(MattermostQt *mattermost READ mattermost WRITE setMattermost NOTIFY mattermostChanged)
	
	friend class UsersModelPrivate;
public:
	explicit UsersModel(QObject *parent = nullptr);
	virtual ~UsersModel();
	
	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	
	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent) const override; 
	
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
	Q_INVOKABLE void request_team_members(int server_index, int team_index);
	Q_INVOKABLE void request_invite_list(int server_index, int team_index, int channel_type, int channel_index);
	
	/** some C++ API */ 
	/**
	 * @brief users 
	 * get current users list
	 * @return 
	 */
	QVector<MattermostQt::UserPtr> users() const;
	
	/**
	 * @brief clear
	 * clear model data 
	 */
	Q_INVOKABLE void clear();
	
protected:
	bool canFetchMore(const QModelIndex &parent) const override;
	void fetchMore(const QModelIndex &parent) override;
	 
Q_SIGNALS:
	void mattermostChanged();
	void request(int type);
	
private Q_SLOTS:
	void userUpdated(MattermostQt::UserPtr user,  QVector<int> roles);
	void userAdded(MattermostQt::UserPtr user);
	
	void onReplyChannelMembers(MattermostQt::ChannelPtr channel, QVector<MattermostQt::UserPtr> members);
	void onReplyTeamMembers(MattermostQt::TeamPtr team, QVector<MattermostQt::UserPtr> members);
private:
	UsersModelPrivate *d;
};

class UsersFilterProxy : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	UsersFilterProxy(QObject *parent = Q_NULLPTR);
protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
//	virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
//	virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
private:
	void slot_sourceModelChanged();
private:
	UsersModel *m_model = nullptr;
};


#endif // USERSMODEL_H
