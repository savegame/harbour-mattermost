#ifndef TEAMSMODEL_H
#define TEAMSMODEL_H

#include <QAbstractListModel>
#include "MattermostQt.h"

class TeamsModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(QString m_serverName READ serverName WRITE setServerName NOTIFY serverNameChanged )
	Q_PROPERTY(int m_serverId READ serverId NOTIFY serverIdChanged)
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
	};

public:
	explicit TeamsModel(QObject *parent = 0);

	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex &index, int role) const;

	QHash<int, QByteArray> roleNames() const;

	Q_INVOKABLE void activate(const int i);

	QString serverName() const;
	void setServerName(QString name);

	int serverId() const;
Q_SIGNALS:
	void serverNameChanged();
	void serverIdChanged();

protected Q_SLOTS:
	void slot_serverConnected(int id);
	void slot_teamAdded(MattermostQt::TeamContainer team);
private:
	QVector<QString> m_displayName;
	QVector<QString> m_description;
	QVector<QString> m_email;
	QVector<QString> m_id;

	QSharedPointer<MattermostQt> m_mattermost;

	int m_serverId;

	QString m_serverName;
};

#endif // TEAMSMODEL_H
