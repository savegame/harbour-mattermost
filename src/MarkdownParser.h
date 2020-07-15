#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>
// Interface class for Markdown parser implementation
class MarkdownParser
{
public:
	enum ParseFlags {
	};

public:
	MarkdownParser();
	
	virtual ~MarkdownParser() {};

	virtual QString parse(const QString &input) = 0;
};

#endif // MARKDOWNPARSER_H
