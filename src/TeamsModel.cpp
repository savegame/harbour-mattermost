#include "TeamsModel.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QString>
#include "MattermostQt.h"


TeamsModel::TeamsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_serverId(-1)
{
	m_mattermost.reset(new MattermostQt());

	connect(m_mattermost.data(), SIGNAL(serverConnected(int)) , SLOT(slot_serverConnected(int)) );
	connect(m_mattermost.data(), &MattermostQt::teamAdded
	        ,this , &TeamsModel::slot_teamAdded );

	m_mattermost->post_login(QString(SERVER_URL),QString("testuser"),QString("testuser"), true);
}

int TeamsModel::rowCount(const QModelIndex &) const
{
	return m_displayName.size();
}

QVariant TeamsModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}
	if(role == DataRoles::DisplayName) {
		return QVariant(m_displayName[index.row()]);
	}
	else if(role == DataRoles::Description) {
		return QVariant(m_description[index.row()]);
	}
	else if(role == DataRoles::Email) {
		return QVariant(m_email[index.row()]);
	}
	return QVariant();
}

QHash<int, QByteArray> TeamsModel::roleNames() const
{
	QHash<int, QByteArray> roleNames;
	roleNames[DataRoles::DisplayName] = QLatin1String("display_name").data();
	roleNames[DataRoles::Description] = QLatin1String("description").data();
	roleNames[DataRoles::Email]       = QLatin1String("email").data();
	return roleNames;
}

void TeamsModel::activate(const int i)
{
	if(i < 0 || i >= m_displayName.size()) {
		return;
	}
//	QString value = backing[i];

//	// Remove the value from the old location.
//	beginRemoveRows(QModelIndex(), i, i);
//	backing.erase(backing.begin() + i);
//	endRemoveRows();

//	// Add it to the top.
//	beginInsertRows(QModelIndex(), 0, 0);
//	backing.insert(0, value);
//	endInsertRows();
}

QString TeamsModel::serverName() const
{
	return m_serverName;
}

void TeamsModel::setServerName(QString name)
{
	m_serverName = name;
	emit serverNameChanged();
}

int TeamsModel::serverId() const
{
	return m_serverId;
}

void TeamsModel::slot_serverConnected(int id)
{
	m_serverId = id;
	m_mattermost->get_teams(id);
	emit serverIdChanged();
}

void TeamsModel::slot_teamAdded(MattermostQt::TeamContainer team)
{
	beginInsertRows(QModelIndex(), m_id.size(), m_id.size());
	m_displayName.append(team.m_display_name);
	m_description.append(team.m_description);
	m_email.append(team.m_email);
	m_id.append(team.m_id);
	endInsertRows();
}
