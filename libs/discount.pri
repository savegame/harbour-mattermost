DEFINES += GITHUB_CHECKBOX

INCLUDEPATH +=	$$PWD/discount-2.2.4
HEADERS += \
	$$PWD/discount-2.2.4/amalloc.h \
	$$PWD/discount-2.2.4/cstring.h \
	$$PWD/discount-2.2.4/gethopt.h \
	$$PWD/discount-2.2.4/markdown.h \
	$$PWD/discount-2.2.4/pgm_options.h \
	$$PWD/discount-2.2.4/tags.h \
	$$PWD/discount-2.2.4/config.h

SOURCES += \
	$$PWD/discount-2.2.4/generate.c\
	$$PWD/discount-2.2.4/resource.c\
	$$PWD/discount-2.2.4/version.c\ 
	$$PWD/discount-2.2.4/toc.c\
	$$PWD/discount-2.2.4/xml.c\ 
	$$PWD/discount-2.2.4/xmlpage.c\
	$$PWD/discount-2.2.4/github_flavoured.c\
	$$PWD/discount-2.2.4/setup.c\
	$$PWD/discount-2.2.4/tags.c\
	$$PWD/discount-2.2.4/html5.c\
	$$PWD/discount-2.2.4/flags.c \
    $$PWD/discount-2.2.4/basename.c \
    $$PWD/discount-2.2.4/Csio.c \
    $$PWD/discount-2.2.4/css.c \
    $$PWD/discount-2.2.4/docheader.c \
    $$PWD/discount-2.2.4/dumptree.c \
    $$PWD/discount-2.2.4/emmatch.c \
    $$PWD/discount-2.2.4/markdown.c \
    $$PWD/discount-2.2.4/mkdio.c

DISTFILES += \
    $$PWD/discount-2.2.4/blocktags
