#include "DiscountMDParser.h"

extern "C" {
#include <markdown.h>
#undef if
#undef while
}

DiscountMDParser::DiscountMDParser()
{
	m_flags = MKD_EXPLICITLIST + MKD_GITHUBTAGS + MKD_FENCEDCODE;
}

QString DiscountMDParser::parse(const QString &input)
{
	Document *doc = gfm_string(input.data(), input.size(), m_flags);

	if (!doc)
	{
		qDebug() << "Error";
	}
	int ok = mkd_compile(doc, flags);

	char *html_text = NULL, *css_text = NULL;
	int size = mkd_document(doc, &html_text);
	int size_2 = mkd_css(doc, &css_text);

	if (size_2)
	{
		qInfo() << css_text;
	}
	if (size)
	{
		QString r = QString::fromUtf8(html_text, size);
//		ui->textArea->setHtml(r);
		ui->htmlArea->setPlainText(r);
//		QQmlProperty(quickText, "text").write(r);
	}
	mkd_cleanup(doc);
}
