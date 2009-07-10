#ifndef _OL_INTL_H_
#define _OL_INTL_H_

/* Internationalization.  */
#include "config.h"
#include "gettext.h"
#define _(String) dgettext(PACKAGE,String)
#define N_(String) (String)

#endif /* _OL_INTL_H_ */
