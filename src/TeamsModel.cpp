#include "TeamsModel.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QString>
#include "MattermostQt.h"


TeamsModel::TeamsModel(QObject *parent)
    : QAbstractListModel(parent)
{
	// login URL
	MattermostQt *mm = new MattermostQt();
	mm->login(QString(SERVER_URL),QString("testuser"),QString("testuser"), true);
}

int TeamsModel::rowCount(const QModelIndex &) const
{
	return backing.size();
}

QVariant TeamsModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}
	if(role == NameRole) {
		return QVariant(backing[index.row()]);
	}
	return QVariant();
}

QHash<int, QByteArray> TeamsModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[NameRole] = QLatin1String("TeamName").data();
}

void TeamsModel::activate(const int i)
{
	if(i < 0 || i >= backing.size()) {
		return;
	}
	QString value = backing[i];

	// Remove the value from the old location.
	beginRemoveRows(QModelIndex(), i, i);
	backing.erase(backing.begin() + i);
	endRemoveRows();

	// Add it to the top.
	beginInsertRows(QModelIndex(), 0, 0);
	backing.insert(0, value);
	endInsertRows();
}
