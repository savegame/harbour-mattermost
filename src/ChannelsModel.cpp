#include "ChannelsModel.h"

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
{
	beginInsertRows(QModelIndex(), 0, 2);
	m_header.resize(3);
	m_display_name.resize(3);
	m_puprose.resize(3);
	m_index.resize(3);
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
		return m_index[index.row()];
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
	m_index.resize(3);
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

	if( channel->m_type.compare("O") == 0 )
		insertIndex = m_header_index[ItemType::HeaderPrivate]++;
	else if( channel->m_type.compare("P") == 0 )
		insertIndex = m_header_index[ItemType::HeaderDirect]++;
	else if( channel->m_type.compare("D") == 0 )
		insertIndex = m_header.size();//m_header_index[ItemType::HeaderDirect] + 1;

	beginInsertRows(QModelIndex(), insertIndex, insertIndex);
	if(insertIndex == m_header.size())
	{
		m_header.append(channel->m_header);
		m_display_name.append(channel->m_display_name);
		m_puprose.append(channel->m_purpose);
		m_type.append(ItemType::Channel);
		m_index.append( channel->m_self_index);
	}
	else
	{
		m_header.insert(insertIndex,channel->m_header);
		m_display_name.insert(insertIndex,channel->m_display_name);
		m_puprose.insert(insertIndex,channel->m_purpose);
		m_type.insert(insertIndex,ItemType::Channel);
		m_index.insert(insertIndex, channel->m_self_index);
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
