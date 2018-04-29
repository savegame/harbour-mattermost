#ifndef TEAMSMODEL_H
#define TEAMSMODEL_H

#include <QAbstractListModel>

class TeamsModel : public QAbstractListModel
{
	Q_OBJECT
public:
	enum DataRoles {
		NameRole = Qt::UserRole + 1,
	};

public:
	explicit TeamsModel(QObject *parent = 0);

	virtual int rowCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex &index, int role) const;

	QHash<int, QByteArray> roleNames() const;

	Q_INVOKABLE void activate(const int i);

private:
	QVector<QString> backing;
};

#endif // TEAMSMODEL_H
