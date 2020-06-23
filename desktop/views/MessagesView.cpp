#include "MessagesView.h"
#include <QScrollBar>
#include <MessagesModel.h>
#include <desktop/views/MessageItemDelegate.h>

MessagesView::MessagesView(QWidget *parent)
    : QListView(parent)
{
//	setSingleStep(80);
	MessagesModel *messagesModel = new MessagesModel();
	messagesModel->setMattermost( MattermostQt::getInstance() );
	ReverseProxyMessagesModel *reverseMessages = new ReverseProxyMessagesModel(messagesModel);
	reverseMessages->setSourceModel(messagesModel);
	setModel(reverseMessages);
	setItemDelegate( new MessageItemDelegate(messagesModel) );
	verticalScrollBar()->setSingleStep(1);
	connect(messagesModel, &MessagesModel::channelChanged, this, &MessagesView::channelChanged );
}

void MessagesView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QListView::currentChanged(current, previous);
}

void MessagesView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
	if( roles.contains(MessagesModel::RoleFileInfo) )
	{
		qWarning() << QStringLiteral("FileNames changed ");
	}
	QListView::dataChanged(topLeft, bottomRight, roles);
}

void MessagesView::rowsInserted(const QModelIndex &parent, int start, int end)
{
	Q_UNUSED(parent)
	Q_UNUSED(start)
	Q_UNUSED(end)
//	if( model()->rowCount() - 1 == end ) // insert to last
	bool scroll = false;
	QModelIndex lastIndex = model()->index(model()->rowCount() - 1,0,parent);
	if( !isIndexHidden( lastIndex ) ) {
//		scrollTo( lastIndex, ScrollHint::EnsureVisible );
		scroll = true;
	}
	QListView::rowsInserted(parent, start, end);
	if(scroll)
		scrollToBottom();
}

void MessagesView::wheelEvent(QWheelEvent *e)
{
//	return QListView::wheelEvent(e);

	QPoint numPixels = e->pixelDelta();
	QPoint numDegrees = e->angleDelta()/4;

	if (!numPixels.isNull()) {
		verticalScrollBar()->setValue(verticalScrollBar()->value() - numPixels.y());
	} else if (!numDegrees.isNull()) {
		verticalScrollBar()->setValue(verticalScrollBar()->value() - numDegrees.y());
	}
	update();
}

void MessagesView::channelChanged()
{
	scrollToBottom();
}
