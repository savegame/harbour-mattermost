#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>

class MarkdownParser
{
public:
	enum ParseFlags {
	};

public:
	MarkdownParser();

	virtual QString parse(const QString &input) = 0;
};

#endif // MARKDOWNPARSER_H
