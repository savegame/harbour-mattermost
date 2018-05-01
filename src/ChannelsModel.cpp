#include "ChannelsModel.h"

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
{
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

//Qt::ItemFlags ChannelsModel::flags(const QModelIndex &index) const
//{
//	if (!index.isValid())
//		return Qt::NoItemFlags;

//	return Qt::ItemIsEnabled; // FIXME: Implement me!
//}

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
}

void ChannelsModel::slot_channelAdded(MattermostQt::ChannelContainer channel)
{
	beginInsertRows(QModelIndex(), m_header.size(), m_header.size());
	m_header.append(channel.m_header);
	m_display_name.append(channel.m_display_name);
	m_puprose.append(channel.m_purpose);
	endInsertRows();
}

//void ChannelsModel::setMattermost(MattermostQt *mattermost)
//{
//	m_mattermost = mattermost;
//}
