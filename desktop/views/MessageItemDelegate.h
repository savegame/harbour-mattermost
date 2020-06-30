#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class MessagesModel;

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

protected:
	int getFileImageHeight(const MessagesModel *m, const QStyleOptionViewItem &option, int message_index, int file_index, int textWidth) const;
};

#endif // MESSAGEITEMDELEGATE_H
