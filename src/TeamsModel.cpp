#include "TeamsModel.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QString>
#include "MattermostQt.h"


TeamsModel::TeamsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_serverId(-1)
{

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
	else if(role == DataRoles::TeamId) {
		return QVariant(m_id[index.row()]);
	}
	else if(role == DataRoles::MsgCount) {
		return QVariant(m_msg_count[index.row()]);
	}
	else if(role == DataRoles::MentionCount) {
		return QVariant(m_mention_count[index.row()]);
	}
	else if(role == DataRoles::ActiveUsers) {
		return QVariant(m_active_users[index.row()]);
	}
	else if(role == DataRoles::UserCount) {
		return QVariant(m_user_count[index.row()]);
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
	return roleNames;
}

void TeamsModel::activate(const int i)
{
	if(i < 0 || i >= m_displayName.size()) {
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

void TeamsModel::setServerId(int id)
{
	m_serverId = id;
	emit serverIdChanged();
}

int TeamsModel::serverId() const
{
	return m_serverId;
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
	//	m_mattermost->post_login(QString(SERVER_URL),QString("testuser"),QString("testuser"), true);
}

QString TeamsModel::getTeamId(int index) const
{
	if(index >= 0 && index < m_id.size())
		return m_id[index];
	return QString::null;
}

//void TeamsModel::slot_serverConnected(int id)
//{
//	m_serverId = id;
//	m_mattermost->get_teams(id);
//	emit serverIdChanged();
//}

void TeamsModel::slot_teamAdded(MattermostQt::TeamPtr team)
{
//	bool noNeed;
//	for(int i = 0; i < m_id.size(); i++)
//	{
//	}
	beginInsertRows(QModelIndex(), m_id.size(), m_id.size());
	m_id.append(team->m_id);
	m_displayName.append(team->m_display_name);
	m_description.append(team->m_description);
	m_email.append(team->m_email);
	m_msg_count.append(0);
	m_mention_count.append(0);
	m_active_users.append(0);
	m_user_count.append(0);
	endInsertRows();
}

void TeamsModel::slot_teamUnread(QString team_id, int msg, int mention)
{
	for(int i = 0; i < m_id.size(); i++)
	{
		if( m_id[i].compare(team_id) == 0 )
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
