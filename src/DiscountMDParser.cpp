#include "DiscountMDParser.h"
#include <QByteArray>
#include <QDebug>

extern "C" {
#include <markdown.h>
#undef if
#undef while
}

DiscountMDParser::DiscountMDParser()
{
	m_flags = MKD_EXPLICITLIST + MKD_GITHUBTAGS /*+ MKD_FENCEDCODE*/;
}

QString DiscountMDParser::parse(const QString &input)
{
	QString result = QString::Null();
	QByteArray utf8 = input.toUtf8();
	Document *doc = gfm_string(utf8.data(), utf8.size(), m_flags);

	if (!doc)
	{
		qWarning() << "Error: cant parse message.";
	}
	int ok = mkd_compile(doc, m_flags);

	char *html_text = NULL/*, *css_text = NULL*/;
	int size = mkd_document(doc, &html_text);
//	int size_2 = mkd_css(doc, &css_text);

//	if (size_2)
//	{
//		qDebug() << css_text;
//	}
	if (size)
		result = QString::fromUtf8(html_text, size);

	mkd_cleanup(doc);
	result = result.left(result.lastIndexOf("<br/>"));
	return result;
}
