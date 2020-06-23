#include "ChannelsModel.h"
//#include "TeamsModel.h"

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
{
	beginInsertRows(QModelIndex(), 0, 2);
	m_header.resize(3);
//	m_display_name.resize(3);
	m_puprose.resize(3);
	m_channel.resize(3);
	m_type.resize(3);
	m_type[0] = ItemType::HeaderPublic;
	m_type[1] = ItemType::HeaderPrivate;
	m_type[2] = ItemType::HeaderDirect;
	m_header_index[ItemType::HeaderPublic] = 0;
	m_header_index[ItemType::HeaderPrivate] = 1;
	m_header_index[ItemType::HeaderDirect] = 2;
	endInsertRows();
}

//QVariant ChannelsModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//	// FIXME: Implement me!
//}

//bool ChannelsModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//	if (value != headerData(section, orientation, role)) {
//		// FIXME: Implement me!
//		emit headerDataChanged(orientation, section, section);
//		return true;
//	}
//	return false;
//}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	// For list models only the root node (an invalid parent) should return the list's size. For all
	// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
//	if (!parent.isValid())
//		return 0;

	return m_header.size();
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	MattermostQt::ChannelPtr channel = m_channel[index.row()];

	switch(role) {
	case Qt::DecorationRole:
		if(channel.isNull() || channel->m_display_name.isEmpty())
			return QVariant("");
		else {
			if( channel->m_type == MattermostQt::ChannelPublic )
				return QVariant::fromValue<QIcon>(QIcon(":/icons/public_channel.svg"));
			else if ( channel->m_type == MattermostQt::ChannelPrivate )
				return QVariant::fromValue<QIcon>(QIcon(":/icons/private_channel.svg"));
			else if ( channel->m_type == MattermostQt::ChannelDirect )
			{
				if( channel->m_dc_user_index < 0 )
					return QVariant("");
				MattermostQt::ServerPtr server = m_mattermost->m_server[channel->m_server_index];
				MattermostQt::UserPtr user = server->m_user[channel->m_dc_user_index];
				return QVariant::fromValue<QIcon>(QIcon(user->m_image_path));
			}
			return QVariant();
		}
	case RoleChannelDescription:
		if( channel ) {
			return channel->m_purpose;
		}
		return QVariant("");
	case DisplayName:
		if(channel.isNull() || channel->m_display_name.isEmpty()) {
			if( m_header_index[ItemType::HeaderPublic] == index.row() )
				return tr("Public Channels");
			else if( m_header_index[ItemType::HeaderPrivate] == index.row() )
				return tr("Private Channels");
			else if( m_header_index[ItemType::HeaderDirect] == index.row() )
				return tr("Direct Channels");
			return QObject::trUtf8("somebody");
		}
		else
			return QVariant(channel->m_display_name);
		break;
	case Purpose:
//		return m_puprose[index.row()];
		if( channel.isNull() )
			return QVariant("");
		return channel->m_purpose;
		break;
	case Email:
		return QVariant();
		break;
	case Header:
//		return m_header[index.row()];
		if( channel.isNull() )
			return QVariant("");
		return channel->m_header;
		break;
	case Type:
		return m_type[index.row()];
		break;
	case ChannelIndex:
		if( channel.isNull() )
			return QVariant(-1);
		return QVariant(channel->m_self_index);
		break;
	case ServerIndex:
		if( channel.isNull() )
			return QVariant(-1);
		return QVariant(channel->m_server_index);
	case TeamIndex:
		if( channel.isNull() )
			return QVariant(-1);
		return QVariant(channel->m_team_index);
		break;
	case ChannelType:
		if( channel.isNull() )
			return QVariant(-1);
		return QVariant((int)channel->m_type);
		break;
	case AvatarPath:
		// show mattermost icon if no Avatar loaded
	    {
		    if(channel.isNull())
				return QVariant("");
			if( channel->m_dc_user_index < 0 )
				return QVariant("");
			if ( channel->m_type != MattermostQt::ChannelDirect )
				return QVariant("");
			MattermostQt::ServerPtr server = m_mattermost->m_server[channel->m_server_index];
			MattermostQt::UserPtr user = server->m_user[channel->m_dc_user_index];
			QString path = user->m_image_path;
			return path;
	    }
		break;
	case UserStatus:
		{
			if(channel.isNull())
				return QVariant(0);
			if( channel->m_dc_user_index < 0 )
				return QVariant(0);
			if ( channel->m_type != MattermostQt::ChannelDirect )
				return QVariant(0);
			MattermostQt::ServerPtr server = m_mattermost->m_server[channel->m_server_index];
			MattermostQt::UserPtr user = server->m_user[channel->m_dc_user_index];
			int current_status = user->m_status;
			return QVariant((int)user->m_status);
		}
	case RoleUserFullName:
	    {
		    if(channel.isNull())
				return QString();
			if ( channel->m_type != MattermostQt::ChannelDirect )
				return QString();
			if( channel->m_dc_user_index < 0 )
				return QString();
			MattermostQt::ServerPtr server = m_mattermost->m_server[channel->m_server_index];
			MattermostQt::UserPtr user = server->m_user[channel->m_dc_user_index];
			return QStringLiteral("%0 %1").arg(user->m_first_name).arg(user->m_last_name);
	    }
	case RoleUserNickname:
	    {
		    if(channel.isNull())
				return QString();
			if ( channel->m_type != MattermostQt::ChannelDirect )
				return QString();
			if( channel->m_dc_user_index < 0 )
				return QString();
			MattermostQt::ServerPtr server = m_mattermost->m_server[channel->m_server_index];
			MattermostQt::UserPtr user = server->m_user[channel->m_dc_user_index];
			return user->m_nickname;
	    }
	case MessageUnread :
		{
			if(channel.isNull())
				return QVariant(0);
			return channel->m_msg_unread;
		}
		break;
	case MentionCount :
		{
			if(channel.isNull())
				return QVariant(0);
			return channel->m_mention_count;
		}
		break;
	case RoleUsersTyping: // count of users
		if ( channel.isNull() )
			return QVariant(0);
		return channel->m_user_typing.size();
	case RoleChannelId:
		if ( channel.isNull() )
			return QVariant(0);
		return channel->m_id;
	case RoleServerIndex:
		if ( channel.isNull() )
			return QVariant(0);
		return channel->m_server_index;
	case RoleChannelIndex:
		if ( channel.isNull() )
			return QVariant(0);
		return channel->m_self_index;
	}
	return QVariant();
}

//bool ChannelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//	if (data(index, role) != value) {
//		// FIXME: Implement me!
//		emit dataChanged(index, index,  QVectorInt() << role);
//		return true;
//	}
//	return false;
//}

Qt::ItemFlags ChannelsModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	if( m_type[index.row()] <= ItemType::HeadersCount )
		return Qt::NoItemFlags;
	else
		return Qt::ItemIsEnabled; // FIXME: Implement me!
}

//bool ChannelsModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//	beginInsertRows(parent, row, row + count - 1);
//	// FIXME: Implement me!
//	endInsertRows();
//}

//bool ChannelsModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//	beginRemoveRows(parent, row, row + count - 1);
//	// FIXME: Implement me!
//	endRemoveRows();
//}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
	// thx to @Kaffeine for that optimization (static const)
	static const QHash<int, QByteArray> names = {
	{ DataRoles::DisplayName,     "m_display_name" },
	{ DataRoles::Purpose,         "m_purpose" },
	{ DataRoles::Header,          "m_header" },
	{ DataRoles::Email,           "m_email" },
	{ DataRoles::ChannelIndex,           "m_index" },
	{ DataRoles::Type,            "m_type" },
	{ DataRoles::ServerIndex,     "server_index" },
	{ DataRoles::TeamIndex,       "team_index" },
	{ DataRoles::ChannelType,     "channel_type" },
	{ DataRoles::AvatarPath,      "avatar_path" },
	{ DataRoles::UserStatus,      "user_status" },
	{ DataRoles::MessageUnread,   "role_msg_unread" },
	{ DataRoles::MentionCount,    "role_mention_count" },
	{ DataRoles::RoleUsersTyping, "role_users_typing" },
	{ DataRoles::RoleChannelId,   "role_channel_id" },
	{ DataRoles::RoleServerIndex, "role_server_index" },
	{ DataRoles::RoleChannelIndex,"role_channel_index" }};
	return names;
}

MattermostQt *ChannelsModel::mattermost()
{
	return m_mattermost;
}

void ChannelsModel::setMattermost(MattermostQt *mattermost)
{
	m_mattermost = mattermost;

	connect( m_mattermost.data(), &MattermostQt::channelAdded, this, &ChannelsModel::slot_channelAdded );
	connect( m_mattermost.data(), &MattermostQt::channelsList, this, &ChannelsModel::slot_channelsList );
	connect( m_mattermost.data(), &MattermostQt::updateChannel, this, &ChannelsModel::slot_updateChannel );
	connect( m_mattermost.data(), &MattermostQt::usersUpdated, this, &ChannelsModel::slot_usersUpdated );
	connect( m_mattermost.data(), &MattermostQt::userUpdated, this, &ChannelsModel::slot_userUpdated );
}

QString ChannelsModel::teamIcon() const
{
	if( !m_team )
		return QString();
	return m_team->m_image_path;
}

void ChannelsModel::clear()
{
	if( m_header.size() == 3 )
		return;
	beginResetModel();
	m_header.clear();
//	m_display_name.clear();
	m_puprose.clear();
	m_type.clear();
//	m_index.resize(3);
	endResetModel();

	beginInsertRows(QModelIndex(), 0, 2);
	m_header.resize(3);
//	m_display_name.resize(3);
	m_puprose.resize(3);
	m_channel.resize(3);
	m_type.resize(3);
	m_type[0] = ItemType::HeaderPublic;
	m_type[1] = ItemType::HeaderPrivate;
	m_type[2] = ItemType::HeaderDirect;
	m_header_index[ItemType::HeaderPublic] = 0;
	m_header_index[ItemType::HeaderPrivate] = 1;
	m_header_index[ItemType::HeaderDirect] = 2;
	endInsertRows();
}

void ChannelsModel::slot_channelAdded(MattermostQt::ChannelPtr channel)
{
	int insertIndex = m_header.size();

	switch( channel->m_type )
	{
	case MattermostQt::ChannelPublic:
		insertIndex = m_header_index[ItemType::HeaderPrivate]++;
		m_header_index[ItemType::HeaderDirect]++;
//		if( !m_team ) {
//			m_team = m_mattermost->teamAt(channel->m_server_index, channel->m_team_index);
//			connect( m_mattermost.data(), &MattermostQt::teamChanged, this, &ChannelsModel::slot_teamChanged );
//		}
		break;
	case MattermostQt::ChannelPrivate:
		insertIndex = m_header_index[ItemType::HeaderDirect]++;
		break;
	case MattermostQt::ChannelDirect:
		insertIndex = m_header.size();
		break;
//	case MattermostQt::ChannelTypeCount:
	default:
		break;
	}

	beginInsertRows(QModelIndex(), insertIndex, insertIndex);
	if(insertIndex == m_header.size())
	{
		m_header.append(channel->m_header);
//		m_display_name.append(channel->m_display_name);
		m_puprose.append(channel->m_purpose);
		m_type.append(ItemType::Channel);
		m_channel.append( channel);
	}
	else
	{
		m_header.insert(insertIndex,channel->m_header);
//		m_display_name.insert(insertIndex,channel->m_display_name);
		m_puprose.insert(insertIndex,channel->m_purpose);
		m_type.insert(insertIndex,ItemType::Channel);
		m_channel.insert(insertIndex, channel);
	}
	endInsertRows();
}

void ChannelsModel::slot_channelsList(QList<MattermostQt::ChannelPtr> list)
{
	clear();
	foreach(MattermostQt::ChannelPtr channel, list)
	{
	// need optimize here, more effective code
		slot_channelAdded(channel);
	}
}

void ChannelsModel::slot_updateChannel(MattermostQt::ChannelPtr channel,  QVectorInt roles)
{
	int headerIndex = 0;
	int endIndex = m_channel.size();

	switch( channel->m_type )
	{
	case MattermostQt::ChannelPublic:
		headerIndex = m_header_index[ItemType::HeaderPublic] + 1;
		endIndex == m_header_index[ItemType::HeaderPrivate];
		break;
	case MattermostQt::ChannelPrivate:
		headerIndex = m_header_index[ItemType::HeaderPrivate] + 1;
		endIndex == m_header_index[ItemType::HeaderDirect];
		break;
	case MattermostQt::ChannelDirect:
		headerIndex = m_header_index[ItemType::HeaderDirect] + 1;
		endIndex = m_channel.size();
		break;
	// case MattermostQt::ChannelTypeCount:
	default:
		headerIndex = m_header_index[ItemType::HeaderDirect] + 1;
		endIndex = m_channel.size();
		break;
	}
	// Q_ASSERT( endIndex < 0 || endIndex > m_channel.size() );
	if( endIndex < 0 || endIndex > m_channel.size() ) {
		qCritical() << QStringLiteral("Somethig went worng! Reset whole model");
		beginResetModel();
		endResetModel();
		return;
	}// TODO change computing of index (from real channel index + nype of channel (favorite-1,public-2,private-3,direct-4) )
	for(int i = headerIndex; i < endIndex; i++)
	{
		if(m_channel[i] == channel)
		{
			headerIndex = i;
			break;
		}
	}
	QModelIndex ci = index(headerIndex);
	dataChanged(ci,ci,roles);
}

void ChannelsModel::slot_usersUpdated(QVector<MattermostQt::UserPtr> users,  QVectorInt roles)
{
	int headerIndex = m_header_index[ItemType::HeaderDirect] + 1;
	int endIndex = m_header.size() - 1;

	QModelIndex ibegin = index(headerIndex);
	QModelIndex iend = index(endIndex);
	dataChanged(ibegin,iend,roles);
}

void ChannelsModel::slot_userUpdated(MattermostQt::UserPtr user,  QVectorInt roles)
{
	int headerIndex = m_header_index[ItemType::HeaderDirect] + 1;
	int endIndex = m_header.size();

	for(int i = headerIndex; i < endIndex; i++)
	{
		if(m_channel[i]->m_dc_user_index == user->m_self_index)
		{
			headerIndex = i;
			break;
		}
	}

	QModelIndex ibegin = index(headerIndex);
	dataChanged(ibegin,ibegin,roles);
}

void ChannelsModel::slot_teamChanged(MattermostQt::TeamPtr team, QVectorInt roles)
{
	// TODO
	if( m_team != team )
		return;
//	if( roles.contains(TeamsModel::) )
}

//void ChannelsModel::setMattermost(MattermostQt *mattermost)
//{
//	m_mattermost = mattermost;
//}

ChannelsFilterProxy::ChannelsFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

bool ChannelsFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
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
//	QRegExp shieldExpSymbols("([\\\.\*\+\[\]\(\)\?])");
//	int pos = shieldExpSymbols.indexIn(pattern, pos);
//	if( pos != -1 && shieldExpSymbols.captureCount() > 0) {
//		for( int i = 1; i < shieldExpSymbols.captureCount(); i++ ) {
//			shieldExpSymbols.cap(1);
//		}
//	}
	regEx.setPattern( QStringLiteral(".*%0.*").arg(pattern) );
	return index0.data(ChannelsModel::DataRoles::Type).toInt() == ChannelsModel::ItemType::Channel &&
	    (
	        index0.data(ChannelsModel::DataRoles::DisplayName).toString().toLower().contains( regEx )
	    );
}

void ChannelsFilterProxy::slot_sourceModelChanged()
{
	m_model = qobject_cast<ChannelsModel*>(sourceModel());
}
