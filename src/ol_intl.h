#ifndef _OL_INTL_H_
#define _OL_INTL_H_

/* Internationalization.  */
#include "gettext.h"
#define _(str) gettext (str)
#define N_(str) gettext_noop (str)

#endif /* _OL_INTL_H_ */
