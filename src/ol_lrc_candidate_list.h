#ifndef _OL_LRC_CANDIDATE_LIST_H_
#define _OL_LRC_CANDIDATE_LIST_H_

#include <gtk/gtk.h>
#include "ol_lrc_fetch.h"

void ol_lrc_candidate_list_init (GtkTreeView *list,
                                 GCallback select_change_callback);
void ol_lrc_candidate_list_set_list (GtkTreeView *list,
                                     const OlLrcCandidate *candidates,
                                     int count);
gboolean ol_lrc_candidate_list_get_selected (GtkTreeView *list,
                                             OlLrcCandidate *candidate);
void ol_lrc_candidate_list_clear (GtkTreeView *list);

#endif /* _OL_LRC_CANDIDATE_LIST_H_ */
