#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QPointer>
#include "MattermostQt.h"

class ChannelsModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(MattermostQt *mattermost READ mattermost WRITE setMattermost)
public:
	enum DataRoles {
		DisplayName = Qt::UserRole + 1,
		Purpose,
		Email,
		Header,
		Type,
		Index,
		Enabled
	};

	enum ItemType {
		HeaderFavourites = 0,
		HeaderPublic,
		HeaderPrivate,
		HeaderDirect,
		HeadersCount,
		Channel
	};
	Q_ENUMS(ItemType)

public:
	explicit ChannelsModel(QObject *parent = nullptr);

	// Header:
//	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

//	// Editable:
//	bool setData(const QModelIndex &index, const QVariant &value,
//	             int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	QHash<int, QByteArray> roleNames() const;
	// Add data:
//	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	// Remove data:
//	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	MattermostQt *mattermost();
	void setMattermost(MattermostQt *mattermost);

protected:
	void clear();

protected Q_SLOTS:
	void slot_channelAdded(MattermostQt::ChannelPtr channel);
	void slot_channelsList(QList<MattermostQt::ChannelPtr> list);
private:
	QVector<QString> m_display_name;
	QVector<QString> m_header;
	QVector<QString> m_puprose;
	QVector<int>     m_index;
	QVector<int>     m_type;
//	QVector<QString> m
	// indexes of headers
	int m_header_index[HeadersCount];
	QPointer<MattermostQt> m_mattermost;
};

#endif // CHANNELSMODEL_H
