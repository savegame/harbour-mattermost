#ifndef EMOJIMODEL_H
#define EMOJIMODEL_H

#include <QAbstractListModel>
#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>
#include <QSharedPointer>


// main EmojiModel
class EmojiModel : public QAbstractListModel
{
	Q_OBJECT

//	Q_PROPERTY(MattermostQt *mattermost READ getMattermost WRITE setMattermost)
	Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
	friend class EmojiProxyCategory;
	friend class EmojiProxyList;
public:
	enum DataRoles {
		RoleName = Qt::UserRole,
		RoleImage,
		RoleCategory,
		RoleType,
	};
	Q_ENUM(DataRoles)

	enum ItemType {
		ItemTypeEmoji,
		ItemTypeCategory,
	};
	Q_ENUM(ItemType)

private:
	struct Item {
		QString  name; // name
		QString  image; // icon
		QString  category; // category
		int      category_index; // index in m_categories
		ItemType type = ItemTypeEmoji;
	};
	typedef QSharedPointer<Item> ItemPtr;

	struct IndexRange {
		int begin = -1;
		int end   = -1;

		IndexRange& operator =(const IndexRange &other);
	};

public:
	EmojiModel(QObject *parent = Q_NULLPTR);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	virtual QHash<int, QByteArray> roleNames() const override;

//	void setMattermost(MattermostQt *mattermost);
//	MattermostQt *getMattermost() const;

	QStringList categories() const;

	Q_INVOKABLE QString categoryIcon(QString category) const;

	ItemPtr  getItem(int row) const;
private:
	void loadEmoji();

private Q_SLOTS:
	void onUsedReactionsChanged();

Q_SIGNALS:
	void categoriesChanged();
	void lastUsedChanged(int from, int to);

private:
//	MattermostQt *m_mattermost = nullptr;
	QVector<ItemPtr>                       m_items;     // indexed list of emoji
	QHash<QString, ItemPtr>                m_hashItems; // hased list of emoji ( for fast search )
	QVector<ItemPtr>                       m_usedItems;
	QVector< QPair<QString, IndexRange> >  m_categories;
};

class EmojiProxyCategory : public QAbstractProxyModel
{
	Q_OBJECT

	Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
public:
	EmojiProxyCategory(QObject *parent = nullptr);

	Q_INVOKABLE virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	Q_INVOKABLE virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

	virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

	Q_INVOKABLE virtual QModelIndex index(int row, int col, const QModelIndex &parent) const override;
	Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const override;

	Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QString category() const;
	void setCategory(const QString &in_category);

private :
	void updateCategoryIndex();

Q_SIGNALS:
	void categoryChanged();

private Q_SLOTS:
	void lastUsedChanged(int from, int to);

private:
	EmojiModel           *m_emojiModel = nullptr;
	QString               m_category;
	EmojiModel::IndexRange  m_categoryIndex;
};

/**
 * @brief The EmojiProxyList class
 * this proxy show emojies as list, but need set count of images in one line
 * it hat two types of lines, first its Category name. second is line of emojies
 */
class EmojiProxyList : public QAbstractProxyModel
{
	Q_OBJECT

	Q_PROPERTY(int emojiColumnCount READ emojiColumnCount WRITE setEmojiColumnCount NOTIFY emojiColumnCountChanged)
public:
	EmojiProxyList(QObject *parent = nullptr);

	Q_INVOKABLE virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	Q_INVOKABLE virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

	virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

	Q_INVOKABLE virtual QModelIndex index(int row, int col, const QModelIndex &parent) const override;
	Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const override;

	Q_INVOKABLE virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE QVariant getData(int row, int column, int role = Qt::DisplayRole) const;

	Q_INVOKABLE int lineSize( int row ) const;
	Q_INVOKABLE int indexOfCategoryHeader(int categoryIndex) const;

	void setEmojiColumnCount(int count);
	int emojiColumnCount() const;
private:
	void recalcCategories();
Q_SIGNALS:
	void emojiColumnCountChanged();

private:
	EmojiModel                      *m_emojiModel = nullptr;
	int                              m_columnCount = 1;
	QVector<EmojiModel::IndexRange>  m_proxyCategories;
	QVector<EmojiModel::IndexRange>  m_sourceLines; // bing every line to indexes in parent model
};

#endif // EMOJIMODEL_H
