#include "Md4cParser.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <string>
#include <md4c-html.h>
#include <libsailfishsilica/silicatheme.h>
#include <cpphash.h>

Md4cMdParser::Md4cMdParser()
{
	block_size[Block_Null] = Silica::Theme::instance()->fontSizeSmall() + 2;

	for( int h = 1; h < BlockCount; h++ )
		block_size[h] = block_size[h - 1] + INCRASE_FONT;

	QString path = QStringLiteral("%0/emoji.json").arg(EMOJI_PATH);
	QFile file(path);
	file.open(QFile::ReadOnly);
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

	if (!error.errorString().isEmpty())
		qWarning() << error.errorString();
	if (doc.isEmpty())
		return;
	if (!doc.isArray())
		return;

	QJsonArray ar = doc.array();
	for (int i = 0; i < ar.size(); i ++ )
	{
		QJsonObject current = ar[i].toObject();
		QString image_name = current["image"].toString();
		if( image_name.indexOf(QStringLiteral("00")) == 0 ) // if name begins with 00, just remove it
			image_name = image_name.mid(2);
		QString image = QStringLiteral("%0/").arg(EMOJI_PATH) + image_name;
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
	
	
	
#ifndef MD4C_USE_ASCII
	renderer_flags = MD_HTML_FLAG_SKIP_UTF8_BOM;
#else
	renderer_flags = MD_HTML_FLAG_DEBUG;
#endif
//	renderer_flags |=MD_HTML_FLAG_VERBATIM_ENTITIES;
	parser_flags |= MD_FLAG_TABLES;
	parser_flags |= MD_FLAG_TASKLISTS;
	parser_flags |= MD_FLAG_STRIKETHROUGH; 
	parser_flags |= MD_FLAG_PERMISSIVEURLAUTOLINKS;
	parser_flags |= MD_FLAG_PERMISSIVEWWWAUTOLINKS;
	parser_flags |= MD_FLAG_PERMISSIVEEMAILAUTOLINKS;
	parser_flags |= MD_FLAG_PERMISSIVEAUTOLINKS;
	parser_flags |= MD_FLAG_UNDERLINE;
	parser_flags |= MD_FLAG_NOHTMLBLOCKS;
	parser_flags |= MD_FLAG_NOHTMLSPANS;
#ifdef _MSC_VER
#else
#endif
}

Md4cMdParser::~Md4cMdParser()
{
	
}

QString Md4cMdParser::parse(const QString &input)
{
//	QString buff;
	buff.clear();
	block_type = Block_Null;
	is_code_block = false;
	
	QByteArray utf8 = input.toUtf8();
//	utf8 = QString("Hello\nWorld").toUtf8();
	md_html( utf8.data(), utf8.size(), [](const MD_CHAR* text, MD_SIZE size, void *userdata) {
		Md4cMdParser* parser = reinterpret_cast<Md4cMdParser*>(userdata);
		QString part = QString::fromUtf8(text,size);
		if( part[0] == QChar('<') ) {// that mean its some tag, not plain text
			if ( part == QStringLiteral("<h1>") )
				parser->block_type = Block_H1;
			else if( part == QStringLiteral("<h2>") )
				parser->block_type = Block_H2;
			else if( part == QStringLiteral("<h3>") )
				parser->block_type = Block_H3;
			else if( part == QStringLiteral("<h4>") )
				parser->block_type = Block_H4;
			else if( part == QStringLiteral("<h5>") )
				parser->block_type = Block_H5;
			else if ( part.lastIndexOf(QRegExp("</h[1-5]>")) == 0 ) 
				parser->block_type = Block_Null;
			else if( !parser->is_code_block && part.lastIndexOf( QRegExp("<table style=\"background-color:#[0-9a-fA-F]{6}\"><tr><td><span style=\"font-family:monospace; white-space: pre-wrap; color: #[0-9a-fA-F]{6};\">") ) == 0 )
				parser->is_code_block = true;
			else if( !parser->is_code_block && part.lastIndexOf( QRegExp("<span style=\"font-family:monospace; white-space: pre-wrap; background-color:#444444; color: #ffffff;\">") ) == 0 )
				parser->is_code_block = true;
			else if( parser->is_code_block && (part.lastIndexOf(QRegExp("</table>")) >= 0 || part.lastIndexOf(QRegExp("</span  >")) >= 0) ) 
				parser->is_code_block = false;
			
		}
		else if(!parser->is_code_block) // we are not in code block
		// better do it inside md4c, but now use Qt for parse some parts of MD (with emoji)
			for(int i = 0, len = 0; i < part.size(); i++)
			{
				if( part[i] == QChar(':') )
				{
					if( len == 0 )
						len = 1;
					else if ( len == 2 ) // это значит что между двумя двоеточиями нет букв, т.е. это просто повторяющееся двоеточие
						len = 1;
					else{ //поймали слово, пробуем определить
						QString word = part.mid(i - len + 2,len - 2);
//						CppHas::instance()
						QHash<QString,QString>::iterator search = parser->emoji.find(word);
						if( search == parser->emoji.end() ) 
							len = 1; // word not found, try search to next ":"
						else {
							part.replace(i - len + 1,len, QStringLiteral("<img width=\"%0\" height=\"%0\" src=\"%1\"/>").arg(int( parser->block_size[parser->block_type] )).arg(search.value()) );
							len = 0; // word is found, clear len  parametr, while not found another ":"
	//						qWarning() << QStringLiteral("Word is \"%0\"").arg(word);
						}
					}
				}
				if(len != 0)
					len++;
			}
		
		parser->buff.append( part );
	}, this, parser_flags, renderer_flags );
	return buff;
}

