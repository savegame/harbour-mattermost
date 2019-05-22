#include "config.h"

char markdown_version[] = "master" "2.2.4"
#if 4 != 4
		" TAB=4"
#endif
#if USE_AMALLOC
		" DEBUG"
#endif
#if GITHUB_CHECKBOX
		" GITHUB_CHECKBOX"
#endif
		;
