#ifndef MESSAGESVIEW_H
#define MESSAGESVIEW_H

#include <QListView>

class MessagesView : public QListView
{
	Q_OBJECT
public:
	explicit MessagesView(QWidget *parent = nullptr);

protected:
	virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

	virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;

	virtual void rowsInserted(const QModelIndex &parent, int start, int end) override;

	virtual void wheelEvent(QWheelEvent *e) override;

	virtual void mouseMoveEvent(QMouseEvent *e) override;

	virtual void mousePressEvent(QMouseEvent *e) override;
signals:

protected slots:

	void channelChanged();
};

#endif // MESSAGESVIEW_H
