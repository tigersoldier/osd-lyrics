#ifndef _OL_LRC_ENGINE_LIST_H_
#define _OL_LRC_ENGINE_LIST_H_

#include <gtk/gtk.h>
#include "ol_lrc_fetch.h"

void ol_lrc_engine_list_init (GtkWidget *list);
void ol_lrc_engine_list_set_name (GtkWidget *list, 
                                  const char *name);
OlLrcFetchEngine *ol_lrc_engine_list_get_engine (GtkWidget *list);
const char *ol_lrc_engine_list_get_name (GtkWidget *list);

#endif /* _OL_LRC_ENGINE_LIST_H_ */
