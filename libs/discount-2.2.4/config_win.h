/*
 * configuration for markdown, generated Thu Feb  5 23:00:42 MSK 2019
 * by sashikknox@gmail.com
 */
#ifndef __AC_MARKDOWN_D
#define __AC_MARKDOWN_D 1


#define OS_LINUX 0
#define THEME_CF 1
#define HAS_GIT 1
#define DESTRUCTOR  
#define while(x) while( (x) != 0 )
#define if(x) if( (x) != 0 )
#define DWORD unsigned int
#define WORD unsigned short
#define BYTE unsigned char
#define HAVE_BASENAME 1
#define HAVE_LIBGEN_H 1
#define HAVE_STDLIB_H 1
#define HAVE_ALLOCA_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_PWD_H 1
#define HAVE_GETPWUID 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STAT 1
#define HAS_ISSOCK 1
#define HAS_ISCHR 1
#define HAS_ISFIFO 1
#define HAVE_SRANDOM 1
#define INITRNG(x) srandom((unsigned int)x)
#define HAVE_MEMSET 1
#define HAVE_RANDOM 1
#define COINTOSS() (random()&1)
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_FCHDIR 1
#define TABSTOP 2
#define HAVE_MALLOC_H 1
#define PATH_FIND "/usr/bin/find"
#define PATH_SED "/usr/bin/sed"
#define random rand
#define srandom srand

#ifndef strncasecmp
#include <string.h>
#define bzero(p, n) memset(p, 0, n)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#endif/* __AC_MARKDOWN_D */
