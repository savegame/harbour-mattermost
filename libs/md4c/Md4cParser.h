#ifndef MADDYMDPARSER_H
#define MADDYMDPARSER_H

#include <MarkdownParser.h>
#include <QHash>
#include <memory>
#include <md4c.h>

class Md4cMdParser : public MarkdownParser
{
public:
	enum BlockType {
		Block_Null,
		Block_H5,
		Block_H4,
		Block_H3,
		Block_H2,
		Block_H1,
		BlockCount
	};
	
	Md4cMdParser();
	virtual ~Md4cMdParser() override;
	
	virtual QString parse(const QString &input) override;
	
private:
	QString buff;
	BlockType block_type = Block_Null;
	QHash<QString,QString> emoji;
	bool      is_code_block = false;
	unsigned int parser_flags = MD_DIALECT_GITHUB;
	unsigned int renderer_flags;
	int block_size[BlockCount];
};

#endif // MADDYMDPARSER_H
