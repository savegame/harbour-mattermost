#ifndef TEAMSMODEL_H
#define TEAMSMODEL_H

#include <QAbstractListModel>
#include <QPointer>
#include "MattermostQt.h"

class TeamsModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(QString m_serverName READ serverName WRITE setServerName NOTIFY serverNameChanged )
	Q_PROPERTY(int server_index READ serverIndex NOTIFY serverIndexChanged WRITE setServerIndex )
	Q_PROPERTY(MattermostQt* mattermost WRITE setMattermostQt READ getMattermostQt )
public:
	enum DataRoles {
//		"id": "string",
//		"create_at": long long,
//		"update_at": long long,
//		"delete_at": long long,
//		"display_name": "string",
//		"name": "string",
//		"description": "string",
//		"email": "string",
//		"type": "string",
//		"allowed_domains": "string",
//		"invite_id": "string",
//		"allow_open_invite": true
		DisplayName = Qt::UserRole + 1,
		Description,
		Email,
		TeamId,
		MsgCount,
		MentionCount,
		ActiveUsers,
		UserCount,
		Index,
		ServerIndex
	};

public:
	explicit TeamsModel(QObject *parent = 0);

	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex &index, int role) const;

	QHash<int, QByteArray> roleNames() const;

	Q_INVOKABLE void activate(const int i);

	QString serverName() const;
	void setServerName(QString name);

	void setServerIndex(int id);
	int serverIndex() const;

	MattermostQt* getMattermostQt() const;
	void setMattermostQt(MattermostQt *mattermost);

	Q_INVOKABLE QString getTeamId(int index) const;
	Q_INVOKABLE int getTeamIndex(int index) const;
Q_SIGNALS:
	void serverNameChanged();
	void serverIndexChanged();

protected Q_SLOTS:
//	void slot_serverConnected(int id);
	void slot_teamAdded(MattermostQt::TeamPtr team);
	void slot_teamsExists(const QVector<MattermostQt::TeamPtr> &teams);
	void slot_teamUnread(QString team_id, int msg, int mention);
private:
//	QVector<QString> m_displayName;
//	QVector<QString> m_description;
//	QVector<QString> m_email;
	QVector<int>     m_msg_count;
	QVector<int>     m_mention_count;
//	QVector<int>     m_active_users;
//	QVector<int>     m_user_count;
	QVector<MattermostQt::TeamPtr> m_team;
//	QVector<QString> m_id;
	QPointer<MattermostQt> m_mattermost;

	int m_server_index;

	QString m_serverName;
};

#endif // TEAMSMODEL_H
