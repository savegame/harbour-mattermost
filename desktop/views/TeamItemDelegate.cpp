#include "TeamItemDelegate.h"
#include <TeamsModel.h>

#define iconSize 42
#define spacing 6

TeamItemDelegate::TeamItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void TeamItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	painter->save();

	QImage image(index.data(TeamsModel::TeamIcon).toString());
//	image.loadFromData(index.data(Qt::DecorationRole).toByteArray());
//	painter->translate(0, 0);
	painter->drawImage(0, 0, image.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0,0, iconSize, iconSize);

	painter->restore();
}

QSize TeamItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return  QSize(iconSize,iconSize + spacing);
}
