#ifndef TEAMSVIEW_H
#define TEAMSVIEW_H

#include <QObject>
#include <QListView>
#include <QStyledItemDelegate>

class TeamItemDelegate : QStyledItemDelegate
{
	Q_OBJECT
public:
	TeamItemDelegate(QObject *parent = Q_NULLPTR);
};

class TeamsView : public QListView
{
	Q_OBJECT
public:
	explicit TeamsView(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // TEAMSVIEW_H
