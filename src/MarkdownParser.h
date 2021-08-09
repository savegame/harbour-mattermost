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

    virtual QString parse(const QString &input, const QString server_id) = 0;
    virtual void    add_custom_emoji(const QString key, const QString path, const QString server_id) = 0;
    virtual QString find_emoji_path(const QString short_name) const = 0;
};

#endif // MARKDOWNPARSER_H
