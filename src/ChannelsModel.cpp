#include "ChannelsModel.h"

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
{
	beginInsertRows(QModelIndex(), 0, 2);
	m_header.resize(3);
	m_display_name.resize(3);
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

	switch(role) {
	case DisplayName:
		return QVariant(m_display_name[index.row()]);
		break;
	case Purpose:
		return m_puprose[index.row()];
		break;
	case Email:
		return QVariant();
		break;
	case Header:
		return m_header[index.row()];
		break;
	case Type:
		return m_type[index.row()];
		break;
	case Index:
		if( m_channel[index.row()].isNull() )
			return QVariant(-1);
		return QVariant(m_channel[index.row()]->m_self_index);
		break;
	case ServerIndex:
		if( m_channel[index.row()].isNull() )
			return QVariant(-1);
		return QVariant(m_channel[index.row()]->m_server_index);
	case TeamIndex:
		if( m_channel[index.row()].isNull() )
			return QVariant(-1);
		return QVariant(m_channel[index.row()]->m_team_index);
		break;
	case ChannelType:
		if( m_channel[index.row()].isNull() )
			return QVariant(-1);
		return QVariant((int)m_channel[index.row()]->m_type);
		break;
	}
	return QVariant();
}

//bool ChannelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//	if (data(index, role) != value) {
//		// FIXME: Implement me!
//		emit dataChanged(index, index, QVector<int>() << role);
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
	QHash<int, QByteArray> roleNames;
	roleNames[DataRoles::DisplayName] = QLatin1String("m_display_name").data();
	roleNames[DataRoles::Purpose] = QLatin1String("m_purpose").data();
	roleNames[DataRoles::Header]  = QLatin1String("m_header").data();
	roleNames[DataRoles::Email]  = QLatin1String("m_email").data();
	roleNames[DataRoles::Index]  = QLatin1String("m_index").data();
	roleNames[DataRoles::Type]  = QLatin1String("m_type").data();
	roleNames[DataRoles::ServerIndex]  = QLatin1String("server_index").data();
	roleNames[DataRoles::TeamIndex]  = QLatin1String("team_index").data();
	roleNames[DataRoles::ChannelType]  = QLatin1String("channel_type").data();
	return roleNames;
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
}

void ChannelsModel::clear()
{
	if( m_header.size() == 3 )
		return;
	beginResetModel();
	m_header.clear();
	m_display_name.clear();
	m_puprose.clear();
	m_type.clear();
//	m_index.resize(3);
	endResetModel();

	beginInsertRows(QModelIndex(), 0, 2);
	m_header.resize(3);
	m_display_name.resize(3);
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
		m_display_name.append(channel->m_display_name);
		m_puprose.append(channel->m_purpose);
		m_type.append(ItemType::Channel);
		m_channel.append( channel);
	}
	else
	{
		m_header.insert(insertIndex,channel->m_header);
		m_display_name.insert(insertIndex,channel->m_display_name);
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

//void ChannelsModel::setMattermost(MattermostQt *mattermost)
//{
//	m_mattermost = mattermost;
//}
