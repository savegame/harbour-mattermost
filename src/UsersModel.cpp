#include "UsersModel.h"
#include <QImage>

class UsersModelPrivate {
public:
	UsersModelPrivate(UsersModel *parent) {
		this->parent = parent;
	}
	
	struct Indexes {
		int server_index  = -1;
		int team_index    = -1;
		int channel_type  = -1;
		int channel_index = -1;
	} indexes;
	
	void checkRequestsStack(QVector<MattermostQt::UserPtr> &members);
	
	void clear() {
		current = UsersModel::NoState;
		finish  = UsersModel::NoState;
		indexes.server_index  = -1;
		indexes.team_index    = -1;
		indexes.channel_type  = -1;
		indexes.channel_index = -1;
		users.clear();
		checkState.clear();
		temp_users.clear();
		request_page = 0;
	}
	
	MattermostQt *m_mattermost = nullptr;
	QVector<MattermostQt::UserPtr> users;
	QVector<Qt::CheckState>        checkState;
	QVector<MattermostQt::UserPtr> temp_users;
	UsersModel::State current = UsersModel::NoState;
	UsersModel::State finish  = UsersModel::NoState;
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
	case Qt::CheckStateRole:
		return  d->checkState[index.row()];
	case MattermostQt::UserNameRole:
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
	case MattermostQt::UserLoginNameRole:
		return user->m_username;
	case MattermostQt::UserImageRole:
		return user->m_image_path;
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
	case MattermostQt::UserIdRole:
		return user->m_id;
	case MattermostQt::UserIsCurrentRole:
		{
			MattermostQt::ServerPtr server = d->m_mattermost->serverAt(d->indexes.server_index);
			return server && server->m_current_user && server->m_current_user->m_id == user->m_id;
		}
	case MattermostQt::UserStatusRole:
		return user->m_status;
	}

	// FIXME: Implement me!
	return QVariant();
}

QHash<int, QByteArray> UsersModel::roleNames() const
{
	static const QHash<int, QByteArray> names = {
	    { MattermostQt::UserNameRole,    "role_user_name"  },
	    { MattermostQt::UserImageRole,   "role_image"      },
	    { MattermostQt::UserIdRole,      "role_user_id"    },
	    { MattermostQt::UserStatusRole,  "role_user_status"}};
	return names;
}

bool UsersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role == Qt::CheckStateRole) {
		if(index.row() >=0 && index.row() < d->checkState.size()) 
		{
			MattermostQt::UserPtr user = d->users[index.row()];
			MattermostQt::ServerPtr server = d->m_mattermost->serverAt(d->indexes.server_index);
			if(server && server->m_current_user && server->m_current_user->m_id == user->m_id ) {
				return false;
			}
			d->checkState[index.row()] = Qt::CheckState(value.toInt());
		}
		return  true;
	}
	return QAbstractListModel::setData(index,value,role);
}

bool UsersModel::insertRows(int row, int count, const QModelIndex &parent)
{
	return false;
	beginInsertRows(parent, row, row + count - 1);
	// FIXME: Implement me!
	endInsertRows();
}

bool UsersModel::removeRows(int row, int count, const QModelIndex &parent)
{
	return false;
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
	if(d->finish == UsersModel::NoState) {
		d->clear();
		d->finish = UsersModel::ChannelMembers;
	}
	if(server_index >= 0) {
		d->indexes.server_index = server_index;
		d->indexes.team_index = team_index;
		d->indexes.channel_type = channel_type;
		d->indexes.channel_index = channel_index;
	}
	d->current = UsersModel::ChannelMembers;
	int page = 0;
	MattermostQt::ChannelPtr channel = mattermost()->channelAt(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index);
	if( channel && d->users.size() < channel->m_member_count ) {
		page = d->request_page; //d->users.size() / d->request_per_page;
	}
	mattermost()->get_channel_members(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index, page, d->request_per_page);
	disconnect(mattermost(), &MattermostQt::onReplyTeamMembers, this, &UsersModel::onReplyTeamMembers);
	connect(mattermost(), &MattermostQt::onReplyChannelMembers, this, &UsersModel::onReplyChannelMembers);
	connect(mattermost(), &MattermostQt::updateChannel, this, &UsersModel::updateChannel);
	connect(mattermost(), &MattermostQt::userRemovedFromChannel, this, &UsersModel::userRemovedFromChannel);
	connect(mattermost(), &MattermostQt::userAddToChannel, this, &UsersModel::userAddToChannel);
}

void UsersModel::request_team_members(int server_index, int team_index)
{
	if(!mattermost()) {
		qCritical() << QStringLiteral("You try request channel mebers without MattermostQt client. Try use singleton");
		setMattermost(MattermostQt::getInstance());
	}
	if(d->finish == UsersModel::NoState) {
		d->clear();
		d->finish = UsersModel::TeamMembers;
	}
	if(server_index >= 0) {
		d->indexes.server_index = server_index;
		d->indexes.team_index = team_index;
	}
	d->current = UsersModel::TeamMembers;
	int page = 0;
	
	MattermostQt::TeamPtr team = mattermost()->teamAt(server_index, team_index);
	if( team &&  d->users.size() < team->m_active_member_count ) {
		page = d->request_page; //d->users.size() / d->request_per_page;
	}
	
	mattermost()->get_team_members(d->indexes.server_index, d->indexes.team_index, page, d->request_per_page);
	disconnect(mattermost(), &MattermostQt::onReplyChannelMembers, this, &UsersModel::onReplyChannelMembers);
	disconnect(mattermost(), &MattermostQt::updateChannel, this, &UsersModel::updateChannel);
	disconnect(mattermost(), &MattermostQt::userRemovedFromChannel, this, &UsersModel::userRemovedFromChannel);
	connect(mattermost(), &MattermostQt::onReplyTeamMembers, this, &UsersModel::onReplyTeamMembers);
}

void UsersModel::request_invite_list(int server_index, int team_index, int channel_type, int channel_index)
{
	if(!mattermost()) {
		qCritical() << QStringLiteral("You try request channel mebers without MattermostQt client. Try use singleton");
		setMattermost(MattermostQt::getInstance());
	}
	
	d->finish = UsersModel::InviteChannelList;
	if( d->current == UsersModel::ChannelMembers ) 
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
	d->finish = UsersModel::InviteChannelList;
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

UsersModel::State UsersModel::state() const
{
	return UsersModel::State(d->current);
}

Qt::CheckState UsersModel::checkState(int i) const
{
	if( i< 0 || i>= d->checkState.size() )
		return Qt::Unchecked;
	return d->checkState[i];
}

QStringList UsersModel::checkedIds() const
{
	QStringList result;
	for(int i=0; i < d->checkState.size(); i++)
		if(d->checkState[i] == Qt::Checked)
			result << d->users[i]->m_id;
	return result;
}

bool UsersModel::isHasChecked() const
{
	QStringList result;
	for(int i=0; i < d->checkState.size(); i++)
		if(d->checkState[i] == Qt::Checked)
			return true;
	return false;
}

bool UsersModel::canFetchMore(const QModelIndex &parent) const
{
	if( /*parent.row() == d->users.size() - 1 && */d->member_count > d->users.size() ) {
		if( d->current == UsersModel::ChannelMembers ) 
		{
			return true;
		}
	}
	return false;
}

void UsersModel::fetchMore(const QModelIndex &parent)
{
	if( /*parent.row() == d->users.size() - 1 && */d->member_count > d->users.size() ) {
		if( d->current == UsersModel::ChannelMembers ) 
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
			d->finish = UsersModel::ChannelMembers;
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
		QModelIndex dataIndex = index(user_index,0);
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
	QModelIndex dataIndex = index(user_index,0);
	if( current->m_self_index < 0 ) 
	{
		users->operator[](user_index) = user;
	}
	dataChanged(dataIndex,dataIndex,QVectorInt() << MattermostQt::UserImageRole << MattermostQt::UserNameRole);
}

void UsersModel::userRemovedFromChannel(QString user_id, MattermostQt::ChannelPtr channel)
{
	MattermostQt::ChannelPtr c =  mattermost()->channelAt(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index);
	if( channel != c  )
		return;
	if( d->current == ChannelMembers )
	for(int user_index = 0; user_index < d->users.size(); user_index++) 
	{
		if( d->users[user_index]->m_id == user_id ) {
			beginRemoveRows(QModelIndex(),user_index,user_index);
			d->users.remove(user_index);
			d->checkState.remove(user_index);
			endRemoveRows();
			break;
		}
	}
}

void UsersModel::userAddToChannel(QString user_id, MattermostQt::ChannelPtr channel)
{
	MattermostQt::ChannelPtr c =  mattermost()->channelAt(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index);
	if( channel != c  )
		return;
	MattermostQt::ServerPtr server = mattermost()->serverAt(d->indexes.server_index);
	MattermostQt::UserPtr user = mattermost()->id2user(server, user_id);
	if( !user ) {
		user.reset(new MattermostQt::UserContainer());
		user->m_id = user_id;
		user->m_self_index = -2;
		mattermost()->get_user_info(d->indexes.server_index,user_id);
	}
	if( d->current == ChannelMembers ) {
		beginInsertRows(QModelIndex(), d->users.size(),d->users.size());
		d->users.push_back(user);
		d->checkState.push_back(Qt::Unchecked);
		endInsertRows();
	}
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

void UsersModel::updateChannel(MattermostQt::ChannelPtr channel, QVectorInt roles)
{
	MattermostQt::ChannelPtr c =  mattermost()->channelAt(d->indexes.server_index, d->indexes.team_index, d->indexes.channel_type, d->indexes.channel_index);
	if( channel != c || !roles.contains(MattermostQt::ChannelMemberCountRole) )
		return;
	if( d->current != UsersModel::ChannelMembers )
		return;
//	TODO dynamicaly update members list if some user deleted
//	d->clear();
//	d->finish = UsersModel::ChannelMembers;
//	request_channel_members(d->indexes.server_index,
//	                        d->indexes.team_index,
//	                        d->indexes.channel_type,
//	                        d->indexes.channel_index);
}

void UsersModelPrivate::checkRequestsStack(QVector<MattermostQt::UserPtr> &members)
{
	if( current == UsersModel::NoState || finish == UsersModel::NoState )
		return;
	if( finish == current ) 
	{
		if( member_count > users.size() ) 
		{
			QVector<MattermostQt::UserPtr> temp_members;
			temp_members.reserve(members.size());
			for(int i = 0; i < members.size(); i++ )
				if( members[i]->m_delete_at == 0 ) {
					temp_members << members[i];
					checkState << Qt::Unchecked;
				}
			parent->beginInsertRows(QModelIndex(), users.size(), users.size() + temp_members.size() - 1 );
//			users.reserve(users.size() + temp_members.size());
			users.append(temp_members);
			parent->endInsertRows();
		}
		else {
			parent->beginResetModel();
			users.clear();
//			users.reserve(users.size() + members.size());
			for(int i = 0; i < members.size(); i++ )
				if( members[i]->m_delete_at == 0 ) {
					users << members[i];
					checkState << Qt::Unchecked;
				}
			parent->endResetModel();
		}
//		current = NoState;
		finish  = UsersModel::NoState;
		return;
	}
	
	switch (current) {
	case UsersModel::TeamMembers:
		if( finish == UsersModel::InviteChannelList ) {
			if( member_count > temp_users.size() + members.size() ) {
//				temp_users.reserve(temp_users.size() + members.size());
				for(int i = 0; i < members.size(); i++ )
					if( members[i]->m_delete_at == 0 )
						temp_users << members[i];
				users.swap(temp_users);
				request_page++;
				parent->request_team_members(indexes.server_index,
				                             indexes.team_index);
				users.swap(temp_users);
			}
			else
			{
//				members << temp_users;
//				temp_users.reserve(temp_users.size() + members.size());
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
				checkState.fill(Qt::Unchecked, users.size());
				parent->endResetModel();
				current = UsersModel::InviteChannelList;
				finish = UsersModel::NoState;
			}
		}
		break;
	case UsersModel::ChannelMembers:
		if( finish == UsersModel::InviteChannelList ) {
//			users.reserve(users.size() + members.size());
			for(int i = 0; i < members.size(); i++ )
				if( members[i]->m_delete_at == 0 )
				{
					users << members[i];
					checkState << Qt::Unchecked;
				}
			// TODO неверная логика, по какой то 
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
		finish = UsersModel::NoState;
		current = UsersModel::NoState;
		break;
	}
}

UsersFilterProxy::UsersFilterProxy(QObject *parent)
	: QSortFilterProxyModel(parent)
{
	setSortCaseSensitivity(Qt::CaseInsensitive);
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
