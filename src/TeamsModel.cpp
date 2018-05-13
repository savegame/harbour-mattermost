#include "TeamsModel.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QString>
#include "MattermostQt.h"


TeamsModel::TeamsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_server_index(-1)
{

}

int TeamsModel::rowCount(const QModelIndex &) const
{
	return m_team.size();
}

QVariant TeamsModel::data(const QModelIndex &index, int role) const
{
	if(index.row() < 0 || index.row() >= m_team.size()) {
		return QVariant();
	}
	if(role == DataRoles::DisplayName) {
		return QVariant(m_team[index.row()]->m_display_name);
	}
	else if(role == DataRoles::Description) {
		return QVariant(m_team[index.row()]->m_description);
	}
	else if(role == DataRoles::Email) {
		return QVariant(m_team[index.row()]->m_email);
	}
	else if(role == DataRoles::TeamId) {
		return QVariant(m_team[index.row()]->m_id);
	}
	else if(role == DataRoles::MsgCount) {
		return QVariant(m_msg_count[index.row()]);
	}
	else if(role == DataRoles::MentionCount) {
		return QVariant(m_mention_count[index.row()]);
	}
	else if(role == DataRoles::ActiveUsers) {
//		return QVariant(m_team[index.row()]);
		return QVariant(0);
	}
	else if(role == DataRoles::UserCount) {
//		return QVariant(m_team[index.row()]);
		return QVariant(0);
	}
	else if(role == DataRoles::Index) {
		return QVariant(m_team[index.row()]->m_self_index);
	}
	else if(role == DataRoles::ServerIndex) {
		return QVariant(m_team[index.row()]->m_server_index);
	}
	return QVariant();
}

QHash<int, QByteArray> TeamsModel::roleNames() const
{
	QHash<int, QByteArray> roleNames;
	roleNames[DataRoles::DisplayName]  = QLatin1String("display_name").data();
	roleNames[DataRoles::Description]  = QLatin1String("description").data();
	roleNames[DataRoles::Email]        = QLatin1String("email").data();
	roleNames[DataRoles::TeamId]       = QLatin1String("teamid").data();
	roleNames[DataRoles::MsgCount]     = QLatin1String("msg_count").data();
	roleNames[DataRoles::MentionCount] = QLatin1String("mention_count").data();
	roleNames[DataRoles::ActiveUsers]  = QLatin1String("active_users").data();
	roleNames[DataRoles::UserCount]    = QLatin1String("user_count").data();
	roleNames[DataRoles::Index]        = QLatin1String("self_index").data();
	roleNames[DataRoles::ServerIndex]  = QLatin1String("server_index").data();
	return roleNames;
}

void TeamsModel::activate(const int i)
{
	if(i < 0 || i >= m_team.size()) {
		return;
	}

//	m_mattermost->get_team(m_serverId, m_id[i]);
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

void TeamsModel::setServerIndex(int id)
{
	m_server_index = id;
	emit serverIndexChanged();
}

int TeamsModel::serverIndex() const
{
	return m_server_index;
}

MattermostQt *TeamsModel::getMattermostQt() const
{
	return m_mattermost;
}

void TeamsModel::setMattermostQt(MattermostQt* mattermost)
{
	m_mattermost = mattermost;

//	connect(m_mattermost, SIGNAL(serverConnected(int)) , SLOT(slot_serverConnected(int)) );
	connect(m_mattermost.data(), &MattermostQt::teamAdded
	        ,this , &TeamsModel::slot_teamAdded );
	connect(m_mattermost.data(), &MattermostQt::teamUnread
	        , this, &TeamsModel::slot_teamUnread );
	connect(m_mattermost.data(), &MattermostQt::teamsExists
	        , this, &TeamsModel::slot_teamsExists );
	//	m_mattermost->post_login(QString(SERVER_URL),QString("testuser"),QString("testuser"), true);
}

QString TeamsModel::getTeamId(int index) const
{
	if(index >= 0 && index < m_team.size())
		return m_team[index]->m_id;
	return QString::null;
}

int TeamsModel::getTeamIndex(int index) const
{
	if(index >= 0 && index < m_team.size())
		return m_team[index]->m_self_index;
	return -1;
}

//void TeamsModel::slot_serverConnected(int id)
//{
//	m_serverId = id;
//	m_mattermost->get_teams(id);
//	emit serverIdChanged();
//}

void TeamsModel::slot_teamAdded(MattermostQt::TeamPtr team)
{
	beginInsertRows(QModelIndex(), m_team.size(), m_team.size());
	m_msg_count.append(0);
	m_mention_count.append(0);
	m_team.append(team);
	endInsertRows();
}

void TeamsModel::slot_teamsExists(const QVector<MattermostQt::TeamPtr> &teams)
{
	beginResetModel();
	m_team = teams;
	m_msg_count.fill(0,teams.size());
	m_mention_count.fill(0,teams.size());
	endResetModel();
}

void TeamsModel::slot_teamUnread(QString team_id, int msg, int mention)
{
	for(int i = 0; i < m_team.size(); i++)
	{
		if( m_team[i]->m_id.compare(team_id) == 0 )
		{
			bool update = false;
			update = m_msg_count[i] != msg || m_mention_count[i] != mention;
			if(update) {
				beginResetModel();
				m_msg_count[i] = msg;
				m_mention_count[i] = mention;
				endResetModel();
			}
		}
	}
}
