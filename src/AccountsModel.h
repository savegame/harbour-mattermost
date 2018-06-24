#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

#include <QAbstractListModel>
#include <QPointer>
#include "MattermostQt.h"

class AccountsModel : public QAbstractListModel
{
	Q_OBJECT

	enum DataRoles {
		RoleName = Qt::UserRole,
		RoleAddress,
		RoleUsername,
		RoleStatus,
		RoleIcon
	};

	Q_PROPERTY(MattermostQt *mattermost READ mattermost WRITE setMattermost)
public:
	explicit AccountsModel(QObject *parent = nullptr);

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	QHash<int, QByteArray> roleNames() const;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	MattermostQt *mattermost();
	void setMattermost(MattermostQt *mattermost);

protected Q_SLOTS:
	void slotServerAdded(MattermostQt::ServerPtr server);
	void slotServerStateChanged(int server_index, int state);
private:
	QPointer<MattermostQt> m_mattermost;
};

#endif // ACCOUNTSMODEL_H
