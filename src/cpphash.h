#ifndef CPPHASH_H
#define CPPHASH_H

#define INCRASE_FONT 6

class CppHashPrivate;
class CppHash
{
public:
	CppHash();

	static CppHash* instance();

	int find_emoji(const char *emoji_short_name, int size, char **picture_path, int *header);
	const char *find_emoji_path(const char *emoji_short_name, int size);
	int count() const;
private:
	CppHashPrivate *d;
};

#endif // CPPHASH_H
