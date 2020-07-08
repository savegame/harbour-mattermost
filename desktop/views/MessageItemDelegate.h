#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H

#include <QObject>
#include <QSharedPointer>
#include <QTextDocument>
#include <QStyledItemDelegate>

class MessagesModel;
/** for painting messages */
class MessageItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	// cached QTextDocument, scaled images, Links to scaled avatars
	struct MessageCache {
		int viewWidth = 0;
		QTextDocument doc;
		QRect         rect;
		QSize         size;
		int           textWidth = 0;
		QHash<QString, QImage> scaledImages;
		QImage avatar;
		QImage avatarShadow;
	};
	typedef QSharedPointer<MessageCache> MessageCachePtr;
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

private:
	QPixmap  avatar_mask;
	mutable QHash<QString, MessageCachePtr> m_messages;
	mutable QHash<QString, QImage> avatars;
};

#endif // MESSAGEITEMDELEGATE_H
