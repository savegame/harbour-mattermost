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
	QHash<int, QByteArray> roleNames;
	roleNames[RoleName]      = QLatin1String("role_name").data();
	roleNames[RoleAddress]   = QLatin1String("role_url").data();
	roleNames[RoleUsername]  = QLatin1String("role_username").data();
	roleNames[RoleStatus]    = QLatin1String("role_status").data();
	roleNames[RoleIcon]      = QLatin1String("role_icon").data();
	return roleNames;
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
	case RoleAddress:
		return server->m_url;
		break;
	case RoleUsername:
		return QString("not implemented");
		break;
	case RoleStatus:
		return server->m_state;
		break;
	case RoleIcon:
		return QString("");
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
