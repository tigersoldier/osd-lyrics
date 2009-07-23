#include "ol_lrc_fetch.h"
#include "ol_lrc_fetch_sogou.h"

OlLrcFetchEngine *
ol_lrc_fetch_get_engine (const char *name)
{
  /* TODO: Return different engine according the name */
  return ol_lrc_fetch_sogou_engine ();
}

