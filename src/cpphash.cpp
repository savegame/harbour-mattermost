#include "cpphash.h"
#include "c_cpphash.h"
#include <QString>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>
#include <QHash>
#include <QFile>

#ifdef DESKTOP_APP
#	ifndef EMOJI_PATH
#		include <MattermostQt.h>
#		define EMOJI_PATH MattermostQt::getInstance()->emojiPath()
#	endif
#elif DESKTOP_QML
#include <desktop/theme.h>
#else
#include <libsailfishsilica/silicatheme.h>
#endif

class CppHashPrivate
{
public:
	CppHashPrivate();

	static CppHash* instance();

	bool parse_emoji_json(QString path);
	bool parse_emoji_json_new(QString path);
public:
	QByteArray              buffer; //for paste image path in discout lib
	QHash<QString, QString> emoji;  // emoji hash short_name - image_name
	int h0,h1,h2,h3,h4,h5;
};

#define INCRASE_FONT 6

CppHashPrivate::CppHashPrivate() {
	if(!parse_emoji_json_new( QStringLiteral("%0/emoji.json").arg(EMOJI_PATH) ))
		qCritical() << "Cant read emoji.json file! Path:" << QStringLiteral("%0/emoji.json").arg(EMOJI_PATH);
#ifdef DESKTOP_QML
	h0 = Theme::getInstance()->fontSizeMedium() + 2;
#elif DESKTOP_APP
	h0 = 16;
#else
	h0 = Silica::Theme::instance()->fontSizeSmall() + 2;
#endif
	h5 = h0 + INCRASE_FONT;
	h4 = h5 + INCRASE_FONT;
	h3 = h4 + INCRASE_FONT;
	h2 = h3 + INCRASE_FONT;
	h1 = h2 + INCRASE_FONT;
}

CppHash::CppHash()
{
	d = new CppHashPrivate();
}

CppHash *CppHash::instance()
{
	static CppHash *cpphash = new CppHash();
	return cpphash;
}

bool CppHashPrivate::parse_emoji_json(QString path)
{// parse emoji json and add it to hash
	qDebug() << "Start parse emoji.json file";
	//	path = QString().arg(EMOJI_PATH);
	QFile file(path);
	file.open(QFile::ReadOnly);
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

	if (!error.errorString().isEmpty())
		qWarning() << error.errorString();
	if (doc.isEmpty())
		return false;
	if (!doc.isArray())
		return false;

	QJsonArray ar = doc.array();
	for (int i = 0; i < ar.size(); i ++ )
	{
		QJsonObject current = ar[i].toObject();
		QString image_name = current["image"].toString();
		if( image_name.indexOf(QStringLiteral("00")) == 0 ) // if name begins with 00, just remove it
			image_name = image_name.mid(2);
		QString image = QStringLiteral("svg/") + image_name.replace(".png",".svg");
		QRegExp re(".*[a-z]+\\-[a-z]+.*");
		//short_names
		QJsonArray short_names = current["short_names"].toArray();
		for( int j = 0; j < short_names.size(); j++)
		{
			QString short_name = short_names[j].toString();
			if( re.indexIn(short_name) != -1 )
				short_name.replace("-","_");
			emoji.insert( short_name, image );
		}
	}

	qDebug() << "All done!";
	return true;
}

bool CppHashPrivate::parse_emoji_json_new(QString path)
{
	qDebug() << "Start parse emoji.json file";
	//	path = QString().arg(EMOJI_PATH);
	QFile file(path);
	file.open(QFile::ReadOnly);
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

	if (!error.errorString().isEmpty())
		qWarning() << error.errorString();
	if (doc.isEmpty())
		return false;
	if (!doc.isArray())
		return false;

	QJsonArray ar = doc.array();
	for (int i = 0; i < ar.size(); i ++ )
	{
		QJsonObject current = ar[i].toObject();
		QString image_name = current["image"].toString();
//		if( image_name.indexOf(QStringLiteral("00")) == 0 ) // if name bgins with 00, just remove it
//			image_name = image_name.mid(2);
//		QString image = QStringLiteral("svg/") + image_name.replace(".png",".svg");
		QRegExp re(".*[a-z]+\\-[a-z]+.*");
		//short_names
		QJsonArray short_names = current["short_names"].toArray();
		for( int j = 0; j < short_names.size(); j++)
		{
			QString short_name = short_names[j].toString();
//			if( re.indexIn(short_name) != -1 )
//				short_name.replace("-","_");
			emoji.insert( short_name, image_name );
		}
	}

	qDebug() << "All done!";
	return true;
}

int CppHash::find_emoji(const char *emoji_short_name, int size, char **picture_path, int *header)
{
	switch(*header) {
	case 1:
		(*header) = d->h1;
		break;
	case 2:
		(*header) = d->h2;
		break;
	case 3:
		(*header) = d->h3;
		break;
	case 4:
		(*header) = d->h4;
		break;
	case 5:
		(*header) = d->h5;
		break;
	default:
		(*header) = d->h0;
		break;
	}

	QString short_name( QByteArray(emoji_short_name,size) );
	QHash<QString, QString>::iterator it =  d->emoji.find(short_name);
	if ( it != d->emoji.end() )
	{
#ifdef DESKTOP_APP
		QString path_pic = QStringLiteral("file://%0/%1").arg(EMOJI_PATH).arg(it.value());
#else
		QString path_pic = QStringLiteral("%0/%1").arg(EMOJI_PATH).arg(it.value());
#endif
		d->buffer = path_pic.toLocal8Bit();
		*picture_path = d->buffer.data();
		return path_pic.size();
	}
	else
		qDebug() << "No Emoji found: " << short_name;
	return 0;
}

const char *CppHash::find_emoji_path(const char *emoji_short_name, int size)
{
	QString short_name( QByteArray(emoji_short_name,size) );
	QHash<QString, QString>::iterator it =  d->emoji.find(short_name);
	if ( it != d->emoji.end() )
	{
		QString path_pic = QStringLiteral("%0/%1").arg(EMOJI_PATH).arg(it.value());
		d->buffer = path_pic.toLocal8Bit();
		return d->buffer.data();
	}
	else
		qDebug() << "No Emoji found: " << short_name;
	return nullptr;
}

int CppHash::count() const
{
	return d->emoji.size();
}


// C part
int find_emoji(const char *emoji_short_name, int size, char **picture_path, int *header)
{
	return CppHash::instance()->find_emoji(emoji_short_name,size,picture_path,header);
}
