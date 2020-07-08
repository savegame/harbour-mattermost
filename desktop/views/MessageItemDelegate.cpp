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
	QImage mask_img(":/resources/avatar_mask.png");
	avatar_mask = QPixmap::fromImage( mask_img.createAlphaMask() );
}

void MessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	int avatarSize = avatarSZ;// < option.decorationSize.width() ? avatarSZ : option.decorationSize.width();
	int textWidth = opt.rect.width() - avatarSize - spacing * 2;
	int textLeft = avatarSize + spacing * 2;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);// TextAntialiasing, SmoothPixmapTransform)

	MessageCachePtr mcache;
	auto search = m_messages.find( index.data(MessagesModel::PostId).toString() );
	if( search != m_messages.end() )
	{
		mcache = search.value();
	}
	else
	{
		mcache.reset( new MessageCache() );
	}

	//------------ selection background
	if( opt.state & QStyle::State_HasFocus )
	{
		QBrush brush = opt.palette.brush(QPalette::Highlight);
		brush.setColor( QColor( brush.color().red(), brush.color().green(), brush.color().blue(), 30 ) );
		// FILL BACKGROUND
		painter->fillRect( opt.rect, brush );
	}
//	index.data(Qt::ItemDataRole
//	if( index == index.model()->)
	//------------ avatar
	int shift = 0;
	if( mcache->avatar.isNull() )
	{
		QString user_id = index.data(MessagesModel::UserId).toString();
		QImage shadow_img(":/resources/avatar_shadow.png");
		shift = int( float(avatar_mask.size().width() - avatarSZ) * 0.5f );
		auto search = avatars.find( user_id );
		if( search != avatars.end() ) {
			mcache->avatar = search.value();
		}
		else
		{
			QImage image(index.data(MessagesModel::SenderImagePath).toString());
			mcache->avatar = image.scaled(avatarSize,avatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			avatars[user_id] = mcache->avatar;
		}
		mcache->avatarShadow = shadow_img.scaled(avatarSize + shift * 2,avatarSize  + shift * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
	painter->translate(opt.rect.left() + spacing, opt.rect.top() );
	painter->drawImage(0,0, mcache->avatarShadow, 0,0, avatarSize  + shift * 2, avatarSize  + shift * 2);
	painter->setClipRegion( QRegion(avatar_mask) );
	painter->translate(shift, shift);
	painter->drawImage(0, 0, mcache->avatar, 0,0, avatarSize, avatarSize);
	// ------------ username
	painter->translate(avatarSize + spacing + shift, 0);
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
//		QTextDocument textDocument;
//		mcache->doc.setDefaultFont(opt.font);
//		mcache->doc.setDefaultStyleSheet(QStringLiteral("body{color:%0;}; code{color:white;}").arg(opt.palette.color(QPalette::Text).name()) );
//		mcache->doc.setHtml( QStringLiteral("<body>") + index.data(MessagesModel::FormatedText).toString() + QStringLiteral("</body>") );
//		mcache->doc.setTextWidth( textWidth );
		QRect clip(0,0, mcache->doc.size().width(), mcache->doc.size().height());
	//	painter->translate(0, 0);
		mcache->doc.drawContents(painter, clip);
		painter->translate(0, mcache->doc.size().height() + smallSpasing);
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
			MattermostQt::FilePtr file = m->getFile(messageIndex,f);
			if( !file )
				continue;
			MattermostQt::FileType type = MattermostQt::FileType( file->m_file_type );
			if( type != MattermostQt::FileImage && type != MattermostQt::FileAnimatedImage )
			{
				painter->drawText(0, opt.fontMetrics.height() * 0.5 + smallSpasing, m->getFileName(messageIndex,f) );
				resultHeight = opt.fontMetrics.height() + smallSpasing;
			}
			else
			{
				auto search = mcache->scaledImages.find(file->m_id);
				QImage scaled;
				if( search == mcache->scaledImages.end() )
				{
					QString path = m->getValidPath(messageIndex, f);
					QImage img( path );
					int geomH = option.widget->geometry().height() - imageMargin;
					int minSize = ( geomH > textWidth)? textWidth : geomH;
					minSize = ( minSize > maxImageSize ) ? maxImageSize : minSize;
					QSizeF imgSize = m->getItemSize(messageIndex, f, minSize);
					scaled = img.scaled(imgSize.width(),imgSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
					mcache->scaledImages[file->m_id] = scaled;
				}
				else
					scaled = search.value();
				resultHeight = opt.fontMetrics.height();
				painter->drawImage(0, resultHeight , scaled, 0,0, scaled.width(), scaled.height());
				painter->drawText (0, opt.fontMetrics.height() * 0.5 + smallSpasing, file->m_name );
				resultHeight +=  int(scaled.height()) + spacing;
			}
			painter->translate(0, resultHeight);
		}
	}

	painter->restore();
}

QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	int widgetWidth = opt.widget->geometry().width();
	int viewWidth = (opt.rect.width() == 0 ) ? widgetWidth : opt.rect.width();
	int avatarSize = avatarSZ;// < option.decorationSize.width() ? avatarSZ : option.decorationSize.width();
	int textWidth = viewWidth - avatarSize - spacing * 2;
	int resultHeight = 0;

	MessageCachePtr mcache;
	QString message_id = index.data(MessagesModel::PostId).toString();
	auto search = m_messages.find( message_id );
	if( search != m_messages.end() )
	{
		mcache = search.value();
	}
	else
	{
		mcache.reset( new MessageCache() );
		m_messages[message_id] = mcache;
	}

	if( opt.rect.width() != 0 && opt.rect.height() != 0 && mcache->rect == option.rect )
		return mcache->size;

	mcache->rect = option.rect;
	// user name header on message =============================
	resultHeight  += option.fontMetrics.height();
	// message text ============================================
	if( !index.data(MessagesModel::Text).toString().isEmpty() )
	{
		QString formatted_text = index.data(MessagesModel::FormatedText).toString();
		int docTextWidth = int(mcache->doc.textWidth());
		if( mcache->doc.toHtml() != formatted_text || mcache->textWidth != textWidth )
		{
			mcache->doc.setDefaultStyleSheet(QStringLiteral("body{color:%0;}; code{color:white;}").arg(opt.palette.color(QPalette::Text).name()) );
			mcache->doc.setDefaultFont( option.font );
			mcache->doc.setHtml( QStringLiteral("<body>") + formatted_text + QStringLiteral("</body>") );
			mcache->doc.setTextWidth( textWidth );
			mcache->textWidth = textWidth;
			mcache->doc.setDocumentMargin(0);
		}
		resultHeight += int(mcache->doc.size().height());
	}
	else {
		resultHeight += spacing;
	}

	// files ==================================================
	int count = index.data(MessagesModel::RoleFileInfo).toInt();
	if( count > 0 || index.data(MessagesModel::FilesCount).toInt() > 0 ) {
		const ReverseProxyMessagesModel *pm = reinterpret_cast<const ReverseProxyMessagesModel*>(index.model());
		const MessagesModel *m = reinterpret_cast<const MessagesModel*>(pm->sourceModel());
		int messageIndex = index.data(MessagesModel::MessageIndex).toInt();
		for(int f = 0; f < count; f++ )
		{
			MattermostQt::FileType type = MattermostQt::FileType(m->getFileType(messageIndex,f));
			if( type != MattermostQt::FileImage && type != MattermostQt::FileAnimatedImage )
				resultHeight += opt.fontMetrics.height() + spacing;
			else {
				resultHeight += getFileImageHeight(m, opt, messageIndex, f, textWidth);
			}
		}
		resultHeight += 3;
	}

//	qWarning() << "Height :" << mcache->doc.size().height();
	resultHeight = avatarSize > resultHeight ? avatarSize : resultHeight;
	mcache->size.setHeight(resultHeight + spacing);
	mcache->size.setWidth( opt.rect.width() );
	return mcache->size;
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
