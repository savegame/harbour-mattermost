#include "MessageItemDelegate.h"
#include <QTextDocument>
#include <MessagesModel.h>
#include <QDebug>

#define  avatarSZ 42
#define  spacing 6
#define  smallSpasing 3
#define  maxImageSize 300
#define  imageMargin  64

MessageItemDelegate::MessageItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void MessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	int avatarSize = avatarSZ;// < option.decorationSize.width() ? avatarSZ : option.decorationSize.width();
	int textWidth = opt.rect.width() - avatarSize - spacing;
	int textLeft = avatarSize + spacing;

	painter->save();
	//avatar
	QImage image(index.data(MessagesModel::SenderImagePath).toString());
	QImage mask_img(":/resources/avatar_mask.png");
//	region.
	QPixmap mask = QPixmap::fromImage( mask_img.createAlphaMask() );
	int shift = int( float(mask_img.size().width() - avatarSZ) * 0.5f );
	painter->translate(opt.rect.left() - shift, opt.rect.top() - shift);
	painter->setClipRegion( QRegion(mask) );
	painter->translate(shift, shift);
	painter->drawImage(0, 0, image.scaled(avatarSize,avatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0,0, avatarSize, avatarSize);
	//test
	QTextDocument textDocument;
	textDocument.setDefaultFont(opt.font);
	textDocument.setDefaultStyleSheet(QStringLiteral("body{color:%0;}").arg(opt.palette.color(QPalette::Text).name()) );
	textDocument.setHtml( QStringLiteral("<body>") + index.data(MessagesModel::FormatedText).toString() + QStringLiteral("</body>") );
	textDocument.setTextWidth( textWidth );
	QRect clip(0,0, textDocument.size().width(), textDocument.size().height());
	painter->translate(textLeft, 0);
	painter->setRenderHint(QPainter::Antialiasing, true);// TextAntialiasing, SmoothPixmapTransform)
	textDocument.drawContents(painter, clip);
	//files
	int count = index.data(MessagesModel::RoleFileInfo).toInt();
	if( count > 0 ) {
		painter->setClipping(false);
		int messageIndex = index.data(MessagesModel::MessageIndex).toInt();
		const ReverseProxyMessagesModel *pm = reinterpret_cast<const ReverseProxyMessagesModel*>(index.model());
		const MessagesModel *m = reinterpret_cast<const MessagesModel*>(pm->sourceModel());
		painter->translate(0, textDocument.size().height() + smallSpasing);
		QStringList preview = index.data(MessagesModel::FilePreview).toStringList();
//		if(!preview.empty())
		for(auto f = 0; f < count; f++ )
		{
			int resultHeight = 0;
			if( m->getFileType(messageIndex,f) == MattermostQt::FileDocument ) {
				painter->drawText(0,0, m->getFileName(messageIndex,f) );
				resultHeight = 32 + smallSpasing;
			}
			else {
				QString path = m->getValidPath(messageIndex, f);
//				QImage preview( m->getValidPath(messageIndex,f) );
				QImage img( path );
				int geomH = option.widget->geometry().height() - imageMargin;
				int minSize = ( geomH > textWidth)? textWidth : geomH;
				minSize = ( minSize > maxImageSize ) ? maxImageSize : minSize;
				QSizeF imgSize = m->getItemSize(messageIndex, f, minSize);
				resultHeight = opt.fontMetrics.height();
				painter->drawImage(0, resultHeight , img.scaled(imgSize.width(),imgSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0,0, imgSize.width(), imgSize.height());
				painter->drawText(0, spacing + smallSpasing, m->getFileName(messageIndex,f) );
				resultHeight +=  int(imgSize.height()) + spacing;
			}
			painter->translate(0, resultHeight);
		}
	}

	painter->restore();
}

QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int avatarSize = avatarSZ;// < option.decorationSize.width() ? avatarSZ : option.decorationSize.width();
	int textWidth = option.rect.width() - avatarSize - spacing;

	QTextDocument textDocument;
	textDocument.setDefaultFont(option.font);
	textDocument.setHtml( index.data(MessagesModel::FormatedText).toString() );
	textDocument.setTextWidth( textWidth );
	textDocument.setDocumentMargin(0);

	int resultHeight = int(textDocument.size().height());

	// files
	int count = index.data(MessagesModel::RoleFileInfo).toInt();
	if( count > 0 ) {
		const ReverseProxyMessagesModel *pm = reinterpret_cast<const ReverseProxyMessagesModel*>(index.model());
		const MessagesModel *m = reinterpret_cast<const MessagesModel*>(pm->sourceModel());
		int messageIndex = index.data(MessagesModel::MessageIndex).toInt();
		for(int f = 0; f < count; f++ )
		{
			if( m->getFileType(messageIndex,f) == MattermostQt::FileDocument )
				resultHeight += 32 + spacing;
			else {
				resultHeight += getFileImageHeight(m, option, messageIndex, f, textWidth);
			}
		}
		resultHeight += 3;
	}

//	qWarning() << "Height :" << textDocument.size().height();
	resultHeight = avatarSize > resultHeight ? avatarSize : resultHeight;
	return QSize( option.rect.width(), resultHeight + spacing);
}

int MessageItemDelegate::getFileImageHeight(const MessagesModel *m, const QStyleOptionViewItem &option, int message_index, int file_index, int textWidth) const
{
	int height = option.fontMetrics.height();
	int geomH = option.widget->geometry().height() - imageMargin;
	int minSize = ( geomH > textWidth)? textWidth : geomH;
	minSize = ( minSize > maxImageSize ) ? maxImageSize : minSize;
	height +=int(m->getItemSize(message_index, file_index, minSize).height()) + spacing;
	return height;
}
