#ifndef MESSAGESMODEL_H
#define MESSAGESMODEL_H

#include <QAbstractListModel>

#include <QSharedPointer>
#include <QPointer>
#include <QList>
#include <QImage>
#include "MattermostQt.h"

class MessagesModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(MattermostQt *mattermost READ getMattermost WRITE setMattermost)

	enum DataRoles : int {
		Text = Qt::UserRole,
		Type,
		FilesCount,
		Thumbinal,
		FileIcon,
		RowIndex
	};

public:
	explicit MessagesModel(QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	virtual QHash<int, QByteArray> roleNames() const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	void setMattermost(MattermostQt *mattermost);
	MattermostQt *getMattermost() const;

	Q_INVOKABLE int getFileType(int row, int i) const;
	Q_INVOKABLE QString getThumbPath(int row, int i) const;

protected slots:
	void slot_messagesAdded(MattermostQt::ChannelPtr channel);
	void slot_messageAdded(QList<MattermostQt::MessagePtr> messages);
	void slot_messageUpdated(QList<MattermostQt::MessagePtr> messages);
private:
	QString                            m_channel_id;
	QVector<MattermostQt::MessagePtr>  m_messages;
	QVector<MattermostQt::UserPtr>     m_users;
	QPointer<MattermostQt>             m_mattermost;
};

#endif // MESSAGESMODEL_H