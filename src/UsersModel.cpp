#include "UsersModel.h"
#include <QImage>

class UsersModelPrivate {
public:
	UsersModelPrivate(UsersModel *parent) {
		this->parent = parent;
	}
	
	enum State {
		NoState,
		ChannelMembers,
		TeamMembers,
		InviteChannelList
	};
	
	struct Indexes {
		int server_index  = -1;
		int team_index    = -1;
		int channel_type  = -1;
		int channel_index = -1;
	} indexes;
	
	void checkRequestsStack(QVector<MattermostQt::UserPtr> &members);
	
	void clear() {
		current = NoState;
		finish  = NoState;
		indexes.server_index  = -1;
		indexes.team_index    = -1;
		indexes.channel_type  = -1;
		indexes.channel_index = -1;
		users.clear();
		temp_users.clear();
	}
	
	MattermostQt *m_mattermost = nullptr;
	QVector<MattermostQt::UserPtr> users;
	QVector<MattermostQt::UserPtr> temp_users;
	State current = NoState;
	State finish  = NoState;
	UsersModel *parent = nullptr;
	int member_count = -1;
	int request_page = 0;
	int request_per_page = MM_DEFAULT_PER_PAGE;
};

UsersModel::UsersModel(QObject *parent)
    : QAbstractListModel(parent)
{
	d = new UsersModelPrivate(this);
}

UsersModel::~UsersModel()
{
	delete d;
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
	return d->users.size();
}

int UsersModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

QVariant UsersModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= d->users.size() || index.row() < 0 )
		return QVariant();
	
	MattermostQt::UserPtr user = d->users[index.row()];
	
	if( index.column() == 1 ) {
		if( role == Qt::DisplayRole )
			return tr("Remove user");
		return QVariant();
	}
		
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
	return d->m_mattermost;
}

void UsersModel::setMattermost(MattermostQt *m)
{
	d->m_mattermost = m;
	connect(mattermost(), &MattermostQt::userAdded, this, &UsersModel::userAdded);
	connect(mattermost(), &MattermostQt::userUpdated, this, &UsersModel::userUpdated);
	emit mattermostChanged();
}

void UsersModel::request_channel_members(int server_index, int team_index, int channel_type, int channel_index)
{
	if(!mattermost()) {
		qCritical() << QStringLiteral("You try request channel mebers without MattermostQt client. Try use singleton");
		setMattermost(MattermostQt::getInstance());
	}
	if(d->finish == UsersModelPrivate::NoState) {
		d->clear();
		d->finish = UsersModelPrivate::ChannelMembers;
	}
	if(server_index >= 0) {
		d->indexes.server_index = server_index;
		d->indexes.team_index = team_index;
		d->indexes.channel_type = channel_type;
		d->indexes.channel_index = channel_index;
	}
	d->current = UsersModelPrivate::ChannelMembers;
	int page = 0;
	MattermostQt::ChannelPtr channel = mattermost()->channelAt(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index);
	if( channel && d->users.size() < channel->m_member_count ) {
		page = d->request_page; //d->users.size() / d->request_per_page;
	}
	mattermost()->get_channel_members(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index, page, d->request_per_page);
	disconnect(mattermost(), &MattermostQt::onReplyTeamMembers, this, &UsersModel::onReplyTeamMembers);
	connect(mattermost(), &MattermostQt::onReplyChannelMembers, this, &UsersModel::onReplyChannelMembers);
}

void UsersModel::request_team_members(int server_index, int team_index)
{
	if(!mattermost()) {
		qCritical() << QStringLiteral("You try request channel mebers without MattermostQt client. Try use singleton");
		setMattermost(MattermostQt::getInstance());
	}
	if(d->finish == UsersModelPrivate::NoState) {
		d->clear();
		d->finish = UsersModelPrivate::TeamMembers;
	}
	if(server_index >= 0) {
		d->indexes.server_index = server_index;
		d->indexes.team_index = team_index;
	}
	d->current = UsersModelPrivate::TeamMembers;
	int page = 0;
	
	MattermostQt::TeamPtr team = mattermost()->teamAt(server_index, team_index);
	if( team &&  d->users.size() < team->m_active_member_count ) {
		page = d->request_page; //d->users.size() / d->request_per_page;
	}
	
	mattermost()->get_team_members(d->indexes.server_index, d->indexes.team_index, page, d->request_per_page);
	disconnect(mattermost(), &MattermostQt::onReplyChannelMembers, this, &UsersModel::onReplyChannelMembers);
	connect(mattermost(), &MattermostQt::onReplyTeamMembers, this, &UsersModel::onReplyTeamMembers);
}

void UsersModel::request_invite_list(int server_index, int team_index, int channel_type, int channel_index)
{
	if(!mattermost()) {
		qCritical() << QStringLiteral("You try request channel mebers without MattermostQt client. Try use singleton");
		setMattermost(MattermostQt::getInstance());
	}
	
	d->finish = UsersModelPrivate::InviteChannelList;
	if( d->current == UsersModelPrivate::ChannelMembers ) 
	{
		if( canFetchMore(QModelIndex()) ) 
		{
			d->request_page++;
			request_channel_members(server_index, team_index, channel_type, channel_index);
		}
		else {
			d->request_page = 0;
			request_team_members(server_index, team_index);
		}
		return;
	}
//	else if( d->current == UsersModelPrivate::TeamMembers ) 
//	{
//		if( canFetchMore(QModelIndex()) ) 
//		{
//			request_team_members(server_index, team_index);
//		}
//		else
//			d->checkRequestsStack(d->users);
//		return;
//	}
	d->clear();
	d->finish = UsersModelPrivate::InviteChannelList;
	request_channel_members(server_index, team_index, channel_type, channel_index);
}

QVector<MattermostQt::UserPtr> UsersModel::users() const
{
	return d->users;
}

void UsersModel::clear()
{
	d->clear();
}

bool UsersModel::canFetchMore(const QModelIndex &parent) const
{
	if( /*parent.row() == d->users.size() - 1 && */d->member_count > d->users.size() ) {
		if( d->current == UsersModelPrivate::ChannelMembers ) 
		{
			return true;
		}
	}
	return false;
}

void UsersModel::fetchMore(const QModelIndex &parent)
{
	if( /*parent.row() == d->users.size() - 1 && */d->member_count > d->users.size() ) {
		if( d->current == UsersModelPrivate::ChannelMembers ) 
		{
			int page = 0;
			MattermostQt::ChannelPtr channel = 
			        mattermost()->channelAt(d->indexes.server_index, 
			                                d->indexes.team_index, 
			                                d->indexes.channel_type,
			                                d->indexes.channel_index);
			if( channel && d->users.size() < channel->m_member_count ) {
				d->request_page++;
				page = d->request_page; //d->users.size() / d->request_per_page;
			}
			d->finish = UsersModelPrivate::ChannelMembers;
			mattermost()->get_channel_members(
			            d->indexes.server_index, 
			            d->indexes.team_index, 
			            d->indexes.channel_type, 
			            d->indexes.channel_index, 
			            page, d->request_per_page);
		}
	}
}

void UsersModel::userUpdated(MattermostQt::UserPtr user, QVector<int> roles)
{
	// check if user is in model
	MattermostQt::UserPtr current;
	int user_index = -1;
	for(int i = 0; i < d->users.size(); i++) 
	{
		current = d->users[i];
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
		d->users[user_index] = user;
	}
}

void UsersModel::userAdded(MattermostQt::UserPtr user)
{
	// check if user is in model
	MattermostQt::UserPtr current;
	QVector<MattermostQt::UserPtr> *users = nullptr;
	int user_index = -1;
	for(int i = 0; i < d->users.size(); i++) 
	{
		current = d->users[i];
		if( current->m_id == user->m_id ) {
			user_index = i;
			users = &d->users;
			break;
		}
	}
	if( users == nullptr  ) {
		for(int i = 0; i < d->temp_users.size(); i++) 
		{
			current = d->temp_users[i];
			if( current->m_id == user->m_id ) {
				user_index = i;
				users = &d->temp_users;
				break;
			}
		}
	}
	if( users == nullptr )
		return;
	QModelIndex dataIndex = index(user_index);
	if( current->m_self_index < 0 ) 
	{
		users->operator[](user_index) = user;
	}
	dataChanged(dataIndex,dataIndex,QVectorInt() << MattermostQt::UserImageRole << MattermostQt::UserNameRole);
}

void UsersModel::onReplyChannelMembers(MattermostQt::ChannelPtr channel, QVector<MattermostQt::UserPtr> members)
{
	d->member_count = channel->m_member_count;
	d->checkRequestsStack(members);
}

void UsersModel::onReplyTeamMembers(MattermostQt::TeamPtr team, QVector<MattermostQt::UserPtr> members)
{
	d->member_count = team->m_active_member_count;
	d->checkRequestsStack(members);
}

void UsersModelPrivate::checkRequestsStack(QVector<MattermostQt::UserPtr> &members)
{
	if( current == NoState || finish == NoState )
		return;
	if( finish == current ) 
	{
		if( member_count > users.size() ) 
		{
			parent->beginInsertRows(QModelIndex(), users.size(), users.size() + members.size() - 1 );
			users.reserve(users.size() + members.size());
			for(int i = 0; i < members.size(); i++ )
				if( members[i]->m_delete_at == 0 )
					users << members[i];
			parent->endInsertRows();
		}
		else {
			parent->beginResetModel();
			users.clear();
			users.reserve(users.size() + members.size());
			for(int i = 0; i < members.size(); i++ )
				if( members[i]->m_delete_at == 0 )
					users << members[i];
			parent->endResetModel();
		}
//		current = NoState;
		finish  = NoState;
		return;
	}
	
	switch (current) {
	case TeamMembers:
		if( finish == InviteChannelList ) {
			if( member_count > temp_users.size() + members.size() ) {
				temp_users.reserve(temp_users.size() + members.size());
				for(int i = 0; i < members.size(); i++ )
					if( members[i]->m_delete_at == 0 )
						temp_users << members[i];
				users.swap(temp_users);
				request_page++;
				parent->request_team_members(indexes.server_index,
				                             indexes.team_index);
				users.swap(temp_users);
			}
			else {
//				members << temp_users;
				temp_users.reserve(temp_users.size() + members.size());
				for(int i = 0; i < members.size(); i++ )
					if( members[i]->m_delete_at == 0 )
						temp_users << members[i];
				temp_users.swap(members);
				temp_users.clear();
				QVector<MattermostQt::UserPtr> new_users;
				new_users.reserve((members.size() > users.size()) ? members.size() - users.size() : users.size());
				parent->beginResetModel();
				// here we need exclude channel members from team members 
				for(int i = 0; i < members.size(); i++) {
					bool skip = false;
					for(int j = 0 ; j < users.size(); j++ ) {
						if( users[j].isNull() )
							continue;
						if( users[j]->m_id == members[i]->m_id )
						{
							skip = true;
							users[j].reset();
							break;
						}
					}
					if( skip )
						continue;
					new_users << members[i];
				}
				users = new_users;
				parent->endResetModel();
				current = InviteChannelList;
				finish = NoState;
			}
		}
		break;
	case ChannelMembers:
		if( finish == InviteChannelList ) {
			users.reserve(users.size() + members.size());
			for(int i = 0; i < members.size(); i++ )
				if( members[i]->m_delete_at == 0 )
					users << members[i];
			if ( member_count > users.size() ) {
				parent->request_invite_list(indexes.server_index,
				                             indexes.team_index,
				                            indexes.channel_type,
				                            indexes.channel_index);
			}
			else 
			{
				request_page = 0;
				parent->request_team_members(indexes.server_index,indexes.team_index);
			}
		}
		break;
	default:
		finish = NoState;
		current = NoState;
		break;
	}
}

UsersFilterProxy::UsersFilterProxy(QObject *parent)
	: QSortFilterProxyModel(parent)
{
	
}

bool UsersFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	QModelIndex index0 = sourceModel()->index(source_row, 0, source_parent);
	QRegExp regEx = filterRegExp();
	if( regEx.pattern().isEmpty() )
		return true;
	QString pattern = regEx.pattern().toLower();
	pattern.replace( QStringLiteral("\\"), QStringLiteral("\\\\") );
	pattern.replace( QStringLiteral("+"), QStringLiteral("\\+") );
	pattern.replace( QStringLiteral("."), QStringLiteral("\\.") );
	pattern.replace( QStringLiteral("*"), QStringLiteral("\\*") );
	pattern.replace( QStringLiteral("["), QStringLiteral("\\[") );
	pattern.replace( QStringLiteral("]"), QStringLiteral("\\]") );
	pattern.replace( QStringLiteral("("), QStringLiteral("\\(") );
	pattern.replace( QStringLiteral(")"), QStringLiteral("\\)") );
	pattern.replace( QStringLiteral("?"), QStringLiteral("\\?") );

	regEx.setPattern( QStringLiteral(".*%0.*").arg(pattern) );
	
	QString data = index0.data(Qt::DisplayRole).toString().toLower();
	return data.contains( regEx );
	
//	MattermostQt::UserPtr user = m_model->users()
}

void UsersFilterProxy::slot_sourceModelChanged()
{
	m_model = qobject_cast<UsersModel*>(sourceModel());
}
