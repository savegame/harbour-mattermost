#ifndef MADDYMDPARSER_H
#define MADDYMDPARSER_H

#include <MarkdownParser.h>
#include <QHash>
#include <QMap>
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
	
    virtual QString parse(const QString &input, const QString server_id) override;
    virtual void    add_custom_emoji(const QString key, const QString path, const QString server_id) override;
    virtual QString find_emoji_path(const QString short_name) const override;

	/**
	 * @brief setBlockSize set blocks sizes
	 * @param sizes
	 */
	void setBlockSize(const QVector<int> &sizes);
private:
	QString buff;
	BlockType block_type = Block_Null;
    QString current_server_id;
	QHash<QString,QString> emoji;
    QMap<QString, QHash<QString,QString>*> custom_emoji;
	bool      is_code_block = false;
	unsigned int parser_flags = MD_DIALECT_GITHUB;
	unsigned int renderer_flags;
	int block_size[BlockCount];
};

#endif // MADDYMDPARSER_H
