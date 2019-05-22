#ifndef DISCOUNTMDPARSER_H
#define DISCOUNTMDPARSER_H

#include "MarkdownParser.h"


class DiscountMDParser : public MarkdownParser
{
public:
	DiscountMDParser();

	virtual QString parse(const QString &input) override;

protected:
	quint32 m_flags;
};

#endif // DISCOUNTMDPARSER_H
