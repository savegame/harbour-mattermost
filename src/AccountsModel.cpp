#include "AccountsModel.h"

AccountsModel::AccountsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant AccountsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}

QHash<int, QByteArray> AccountsModel::roleNames() const
{
	static const QHash<int, QByteArray> names ={
	{ RoleName,        "role_name" },
	{ RoleServerUrl,   "role_url" },
	{ RoleUsername,    "role_username" },
	{ RoleStatus,      "role_status" },
	{ RoleIsEnabled,   "role_is_enabled" },
	{ RoleIcon,        "role_icon" },
	{ RoleServerIndex, "role_server_index" } };
	return names;
}

int AccountsModel::rowCount(const QModelIndex &parent) const
{
	// For list models only the root node (an invalid parent) should return the list's size. For all
	// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
	if (m_mattermost.isNull())
		return 0;
	return m_mattermost->server().size();
}

QVariant AccountsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || m_mattermost.isNull())
		return QVariant();

	const MattermostQt::ServerPtr server = m_mattermost->server().at(index.row());

	if(server.isNull())
		return QVariant();

	switch (role) {
	case RoleName:
		return server->m_display_name;
		break;
	case RoleServerUrl:
		return server->m_url;
		break;
	case RoleUsername:
		return QString("not implemented");
		break;
	case RoleStatus:
		return server->m_state;
		break;
	case RoleIsEnabled:
		return server->m_enabled;
		break;
	case RoleIcon:
		return QString("");
		break;
	case RoleServerIndex:
		return server->m_self_index;
		break;
	default:
		break;
	}
	return QVariant();
}

MattermostQt *AccountsModel::mattermost()
{
	return m_mattermost.data();
}

void AccountsModel::setMattermost(MattermostQt *mattermost)
{
	m_mattermost = mattermost;
	connect( m_mattermost.data(), &MattermostQt::serverAdded, this , &AccountsModel::slotServerAdded );
	connect( m_mattermost.data(), &MattermostQt::serverStateChanged, this , &AccountsModel::slotServerStateChanged );
	connect( m_mattermost.data(), &MattermostQt::serverChanged, this , &AccountsModel::slotServerChanged );
}

void AccountsModel::slotServerAdded(MattermostQt::ServerPtr server)
{
	beginInsertRows( QModelIndex(), server->m_self_index, server->m_self_index );
	endInsertRows();
}

void AccountsModel::slotServerStateChanged(int server_index, int state)
{
	QVector<int> roles;
	roles << RoleStatus;
	dataChanged( index(server_index), index(server_index), roles );
}

void AccountsModel::slotServerChanged(MattermostQt::ServerPtr server, QVector<int> roles)
{
	dataChanged( index(server->m_self_index), index(server->m_self_index), roles );
}
