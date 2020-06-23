#ifndef TEAMSVIEW_H
#define TEAMSVIEW_H

#include <QObject>
#include <QListView>
#include <QStyledItemDelegate>

class TeamItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	TeamItemDelegate(QObject *parent = Q_NULLPTR);

	// painting
	void paint(QPainter *painter,
	           const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option,
	               const QModelIndex &index) const override;
};

#endif // TEAMSVIEW_H
