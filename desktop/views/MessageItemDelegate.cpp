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
	painter->setRenderHint(QPainter::Antialiasing, true);// TextAntialiasing, SmoothPixmapTransform)
	//------------ avatar
	QImage image(index.data(MessagesModel::SenderImagePath).toString());
	QImage mask_img(":/resources/avatar_mask.png");
	QImage shadow_img(":/resources/avatar_shadow.png");
//	region.
	QPixmap mask = QPixmap::fromImage( mask_img.createAlphaMask() );
	int shift = int( float(mask_img.size().width() - avatarSZ) * 0.5f );
	painter->translate(opt.rect.left() - shift, opt.rect.top() - shift);
	painter->drawImage(0,0, shadow_img.scaled(avatarSize + shift * 2,avatarSize  + shift * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0,0, avatarSize  + shift * 2, avatarSize  + shift * 2);
	painter->setClipRegion( QRegion(mask) );
	painter->translate(shift, shift);
	painter->drawImage(0, 0, image.scaled(avatarSize,avatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0,0, avatarSize, avatarSize);
	// ------------ username
	painter->translate(textLeft, shift);
	painter->setClipping(false);
	painter->setPen( opt.palette.color(QPalette::Text) );
	QString username = index.data(MessagesModel::RoleSenderFullName).toString();
	QFont bold = opt.font;
	bold.setBold(true);
	painter->setFont(bold);
	painter->drawText (0, opt.fontMetrics.height() * 0.5, username );
	painter->translate(0, opt.fontMetrics.height() * 0.5);
	// ------------ message text
	if( !index.data(MessagesModel::Text).toString().isEmpty() ) {
		QTextDocument textDocument;
		textDocument.setDefaultFont(opt.font);
		textDocument.setDefaultStyleSheet(QStringLiteral("body{color:%0;}; code{color:white;}").arg(opt.palette.color(QPalette::Text).name()) );
		textDocument.setHtml( QStringLiteral("<body>") + index.data(MessagesModel::FormatedText).toString() + QStringLiteral("</body>") );
		textDocument.setTextWidth( textWidth );
		QRect clip(0,0, textDocument.size().width(), textDocument.size().height());
	//	painter->translate(0, 0);
		textDocument.drawContents(painter, clip);
		painter->translate(0, textDocument.size().height() + smallSpasing);
	}
	else
		painter->translate(0, spacing);
	// ----------- attached files
	QFont smaller_font = opt.font;
	smaller_font.setPointSizeF(smaller_font.pointSizeF() * 0.8);
//	smaller_font.setPixelSize(smaller_font.pixelSize() - 2);
	smaller_font.setBold(true);
	painter->setFont(smaller_font);
	int count = index.data(MessagesModel::RoleFileInfo).toInt();
	if( count > 0 ) {
		painter->setClipping(false);
		int messageIndex = index.data(MessagesModel::MessageIndex).toInt();
		const ReverseProxyMessagesModel *pm = reinterpret_cast<const ReverseProxyMessagesModel*>(index.model());
		const MessagesModel *m = reinterpret_cast<const MessagesModel*>(pm->sourceModel());
		QStringList preview = index.data(MessagesModel::FilePreview).toStringList();
//		if(!preview.empty())
		for(auto f = 0; f < count; f++ )
		{
			int resultHeight = 0;
			MattermostQt::FileType type = MattermostQt::FileType(m->getFileType(messageIndex,f));
			if( type != MattermostQt::FileImage && type != MattermostQt::FileAnimatedImage )
			{
				painter->drawText(0, opt.fontMetrics.height() * 0.5 + smallSpasing, m->getFileName(messageIndex,f) );
				resultHeight = opt.fontMetrics.height() + smallSpasing;
			}
			else
			{
				QString path = m->getValidPath(messageIndex, f);
//				QImage preview( m->getValidPath(messageIndex,f) );
				QImage img( path );
				int geomH = option.widget->geometry().height() - imageMargin;
				int minSize = ( geomH > textWidth)? textWidth : geomH;
				minSize = ( minSize > maxImageSize ) ? maxImageSize : minSize;
				QSizeF imgSize = m->getItemSize(messageIndex, f, minSize);
				resultHeight = opt.fontMetrics.height();
				painter->drawImage(0, resultHeight , img.scaled(imgSize.width(),imgSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), 0,0, imgSize.width(), imgSize.height());
				painter->drawText (0, opt.fontMetrics.height() * 0.5 + smallSpasing, m->getFileName(messageIndex,f) );
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
	int resultHeight = 0;

	resultHeight  += option.fontMetrics.height();

	if( !index.data(MessagesModel::Text).toString().isEmpty() )
	{
		QString formatted_text = index.data(MessagesModel::FormatedText).toString();
		QTextDocument textDocument;
		textDocument.setDefaultFont(option.font);
		textDocument.setHtml( formatted_text );
		textDocument.setTextWidth( textWidth );
		textDocument.setDocumentMargin(0);
		resultHeight += int(textDocument.size().height());
	}
	else {
		resultHeight += spacing;
	}

	// files
	int count = index.data(MessagesModel::RoleFileInfo).toInt();
	if( count > 0 ) {
		const ReverseProxyMessagesModel *pm = reinterpret_cast<const ReverseProxyMessagesModel*>(index.model());
		const MessagesModel *m = reinterpret_cast<const MessagesModel*>(pm->sourceModel());
		int messageIndex = index.data(MessagesModel::MessageIndex).toInt();
		for(int f = 0; f < count; f++ )
		{
			MattermostQt::FileType type = MattermostQt::FileType(m->getFileType(messageIndex,f));
			if( type != MattermostQt::FileImage && type != MattermostQt::FileAnimatedImage )
				resultHeight += option.fontMetrics.height() + spacing;
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
