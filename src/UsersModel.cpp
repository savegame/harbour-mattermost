#include "UsersModel.h"
#include <QImage>

UsersModel::UsersModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant UsersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// FIXME: Implement me!
	switch(role) {
	case Qt::DisplayRole:
		return  tr("Users");
	}
}

int UsersModel::rowCount(const QModelIndex &parent) const
{
	return m_users.size();
}

QVariant UsersModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= m_users.size() || index.row() < 0 )
		return QVariant();
	
	MattermostQt::UserPtr user = m_users[index.row()];
	
	switch(role) {
	case Qt::DisplayRole:
		if(user->m_self_index >= 0)
		{
			QString full_name;
			if( user->m_first_name.isEmpty() && user->m_last_name.isEmpty() )
				full_name = user->m_username;
			else 
				full_name = QStringLiteral("%0 %1").arg(user->m_first_name).arg(user->m_last_name);
			return QStringLiteral("%1\n@%2").arg(full_name).arg(user->m_username);
		}
		else 
		{
			return tr("Unknown user");
		}
	case Qt::DecorationRole:
		{
			QImage avatar;
			if( user->m_self_index >= 0 ) 
			{
				if( user->m_image_path.isEmpty() )
					avatar = QImage(":/resources/empty_avatar.png");
				else
					avatar = QImage(user->m_image_path);
			}
			else
				avatar = QImage(":/resources/empty_avatar.png");
			return avatar;
		}
	}

	// FIXME: Implement me!
	return QVariant();
}

bool UsersModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row + count - 1);
	// FIXME: Implement me!
	endInsertRows();
}

bool UsersModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row + count - 1);
	// FIXME: Implement me!
	endRemoveRows();
}

MattermostQt *UsersModel::mattermost() const
{
	return m_mattermost;
}

void UsersModel::setMattermost(MattermostQt *m)
{
	m_mattermost = m;
	
	emit mattermostChanged();
}

void UsersModel::request_channel_members(int server_index, int team_index, int channel_type, int channel_index)
{
	if(!mattermost()) {
		qCritical() << QStringLiteral("You try request channel mebers without MattermostQt client. Try use singleton");
	}
	mattermost()->get_channel_members(server_index, team_index, channel_type, channel_index);
	connect(mattermost(), &MattermostQt::onReplyChannelMembers, this, &UsersModel::onReplyChannelMembers);
	connect(mattermost(), &MattermostQt::userAdded, this, &UsersModel::userAdded);
}

void UsersModel::userUpdated(MattermostQt::UserPtr user, QVector<int> roles)
{
	// check if user is in model
	MattermostQt::UserPtr current;
	int user_index = -1;
	for(int i = 0; i < m_users.size(); i++) 
	{
		current = m_users[i];
		if( current->m_id == user->m_id ) {
			user_index = i;
			break;
		}
	}
	if( user_index == -1  )
		return;
	
	if( current->m_self_index < 0 ) 
	{
		QModelIndex dataIndex = index(user_index);
		dataChanged(dataIndex,dataIndex,roles);
		m_users[user_index] = user;
	}
}

void UsersModel::userAdded(MattermostQt::UserPtr user)
{
	// check if user is in model
	MattermostQt::UserPtr current;
	int user_index = -1;
	for(int i = 0; i < m_users.size(); i++) 
	{
		current = m_users[i];
		if( current->m_id == user->m_id ) {
			user_index = i;
			break;
		}
	}
	if( user_index == -1  )
		return;
	
	QModelIndex dataIndex = index(user_index);
	if( current->m_self_index < 0 ) 
	{
		m_users[user_index] = user;
	}
	dataChanged(dataIndex,dataIndex,QVectorInt() << MattermostQt::UserImageRole << MattermostQt::UserNameRole);
}

void UsersModel::onReplyChannelMembers(QString channel_id, QVector<MattermostQt::UserPtr> members)
{
	Q_UNUSED(channel_id);
	beginResetModel();
	m_users = members;
	endResetModel();
}
