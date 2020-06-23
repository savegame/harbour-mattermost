#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class MessageItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit MessageItemDelegate(QObject *parent = nullptr);

	// painting
	void paint(QPainter *painter,
	           const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option,
	               const QModelIndex &index) const override;
signals:

public slots:

private:

};

#endif // MESSAGEITEMDELEGATE_H
