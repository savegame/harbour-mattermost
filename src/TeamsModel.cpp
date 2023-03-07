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
	return m_server.isNull() ? 0 : m_server->m_teams.size();
}

QVariant TeamsModel::data(const QModelIndex &index, int role) const
{
	if (!m_mattermost)
		return QVariant();

	MattermostQt::ServerPtr server = m_mattermost->serverAt(m_server_index);
	if (!server)
		return QVariant();

	if(index.row() < 0 || index.row() >= server->m_teams.size()) {
		return QVariant();
	}
	MattermostQt::TeamPtr team = server->m_teams[index.row()];

	if(role == DataRoles::DisplayName) {
		return QVariant(team->m_display_name);
	}
	else if(role == DataRoles::Description) {
		return QVariant(team->m_description);
	}
	else if(role == DataRoles::Email) {
		return QVariant(team->m_email);
	}
	else if(role == DataRoles::TeamId) {
		return QVariant(team->m_id);
	}
	else if(role == DataRoles::RoleUnreadMessageCount) {
		return QVariant(team->m_unread_messages);
	}
	else if(role == DataRoles::RoleUnreadMentionCount) {
		return QVariant(team->m_unread_mentions);
	}
	else if(role == DataRoles::ActiveUsers) {
//		return QVariant(team);
		return QVariant(0);
	}
	else if(role == DataRoles::UserCount) {
//		return QVariant(team);
		return QVariant(0);
	}
	else if(role == DataRoles::Index) {
		return QVariant(team->m_self_index);
	}
	else if(role == DataRoles::ServerIndex) {
		return QVariant(team->m_server_index);
	}
	else if(role == DataRoles::ServerCustomName) {
		return QVariant( m_mattermost->get_server_name(m_server_index) );
	}
	else if(role == DataRoles::TeamIcon) {
		return team->m_image_path;
	}
	else if(role == Qt::DecorationRole) {
		if( team->m_image_path.isEmpty() )
			return QIcon();
		else
			return QIcon(team->m_image_path);
	}
	return QVariant();
}

QHash<int, QByteArray> TeamsModel::roleNames() const
{
	// thx to @Kaffeine for that optimization (static const)
	static const QHash<int, QByteArray> names = {
	{ DataRoles::DisplayName,             "role_display_name" },
	{ DataRoles::Description,             "role_description" },
	{ DataRoles::Email,                   "email" },
	{ DataRoles::TeamId,                  "role_team_id" },
	{ DataRoles::RoleUnreadMessageCount,  "role_unread_message" },
	{ DataRoles::RoleUnreadMentionCount,  "role_unread_mention" },
	{ DataRoles::ActiveUsers,             "active_users" },
	{ DataRoles::UserCount,               "user_count" },
	{ DataRoles::Index,                   "role_team_index" },
	{ DataRoles::ServerIndex,             "role_server_index" },
	{ DataRoles::ServerCustomName,        "role_server_display_name" },
	{ DataRoles::TotalMemberCount,        "role_total_member_count" },
	{ DataRoles::ActiveMemberCount,       "role_active_member_count" },
	{ DataRoles::TeamIcon,                "role_team_icon" } };
	return names;
}

void TeamsModel::activate(const int i)
{
	Q_ASSERT(!m_server.isNull());
	if(i < 0 || i >= m_server->m_teams.size()) {
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
	Q_ASSERT(!m_mattermost.isNull());
	m_server = m_mattermost->serverAt(id);
	if (!m_server) {
		qDebug() << QStringLiteral("Wrong server index: %1").arg(id);
		return;
	}
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
	connect(m_mattermost.data(), &MattermostQt::teamChanged
	        , this, &TeamsModel::slot_teamChanged );
	//	m_mattermost->post_login(QString(SERVER_URL),QString("testuser"),QString("testuser"), true);
}

QString TeamsModel::getTeamId(int index) const
{
	Q_ASSERT(!m_server.isNull());

	if(index >= 0 && index < m_server->m_teams.size())
		return m_server->m_teams[index]->m_id;
	return QString::Null();
}

int TeamsModel::getTeamIndex(int index) const
{
	Q_ASSERT(!m_server.isNull());

	if(index >= 0 && index < m_server->m_teams.size())
		return m_server->m_teams[index]->m_self_index;
	return -1;
}

void TeamsModel::waitAllTeams(bool wait)
{
	if(!m_mattermost)
		return;
	if( wait )
		connect(m_mattermost, &MattermostQt::onReplyAllTeams, this, &TeamsModel::setTeamsList );
	else
		disconnect(m_mattermost, &MattermostQt::onReplyAllTeams, this, &TeamsModel::setTeamsList );
}

void TeamsModel::setTeamsList(QList<MattermostQt::TeamPtr> teams)
{
	Q_UNUSED(teams)
	Q_ASSERT(!m_mattermost.isNull() && !m_server.isNull());

	disconnect(m_mattermost, &MattermostQt::onReplyAllTeams, this, &TeamsModel::setTeamsList );
	beginResetModel();
//	m_server->m_teams.clear();
//	for( auto each : teams ) {
//		m_team << each;
//	}
	endResetModel();
}

//void TeamsModel::slot_serverConnected(int id)
//{
//	m_serverId = id;
//	m_mattermost->get_teams(id);
//	emit serverIdChanged();
//}

void TeamsModel::slot_teamAdded(MattermostQt::TeamPtr team)
{
	if (m_server_index != team->m_server_index) {
		return;
	}

	MattermostQt::ServerPtr s = m_mattermost->serverAt(m_server_index);
	if (!s) {

	}
	beginInsertRows(QModelIndex(), s->m_teams.size() - 1, s->m_teams.size() - 1);
//	m_msg_count.append(0);
//	m_mention_count.append(0);
//	m_team.append(team);
	endInsertRows();
}

void TeamsModel::slot_teamsExists(const QVector<MattermostQt::TeamPtr> &teams)
{
	beginResetModel();
//	m_team = teams;
//	m_msg_count.fill(0,teams.size());
//	m_mention_count.fill(0,teams.size());
	endResetModel();
}

void TeamsModel::slot_teamUnread(QString team_id, int msg, int mention)
{
	Q_ASSERT(!m_server.isNull());
	for(int i = 0; i < m_server->m_teams.size(); i++)
	{
		if( m_server->m_teams[i]->m_id.compare(team_id) == 0 )
		{
			QModelIndex tl = index(i);
			dataChanged(tl,tl,QVectorInt() << RoleUnreadMentionCount << RoleUnreadMessageCount);
			break;
		}
	}
}

void TeamsModel::slot_teamChanged(MattermostQt::TeamPtr team, QVectorInt roles)
{
	Q_ASSERT(!m_server.isNull());
	for(int i = 0; i < m_server->m_teams.size(); i++)
	{
		if( m_server->m_teams[i] == team )
		{
			QModelIndex tl = index(i);
			dataChanged(tl,tl,roles);
			break;
		}
	}
}
