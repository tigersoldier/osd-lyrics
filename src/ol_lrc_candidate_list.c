#include "ol_lrc_candidate_list.h"
#include "ol_intl.h"
#include "ol_debug.h"

enum {
  TITLE_COLUMN = 0,
  ARTIST_COLUMN,
  URL_COLUMN,
  COLUMN_COUNT,
};

void
ol_lrc_candidate_list_init (GtkTreeView *list,
                            GCallback select_changed_callback)
{
  ol_assert (list != NULL);
  GtkTreeStore *store = gtk_tree_store_new (COLUMN_COUNT,    /* Total number of columns */
                                            G_TYPE_STRING,   /* Music title             */
                                            G_TYPE_STRING,   /* Author                  */
                                            G_TYPE_STRING);  /* URL                     */
  
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Title"),
                                                     renderer,
                                                     "text", TITLE_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (list, column);
  column = gtk_tree_view_column_new_with_attributes (_("Artist"),
                                                     renderer,
                                                     "text", ARTIST_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (list, column);
  gtk_tree_view_set_model (list, GTK_TREE_MODEL (store));

  GtkTreeSelection *select;
  select = gtk_tree_view_get_selection (list);
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  if (select_changed_callback != NULL)
  {
    g_signal_connect (G_OBJECT (select), "changed",
                      G_CALLBACK (select_changed_callback),
                      NULL);
  }
}

void
ol_lrc_candidate_list_set_list (GtkTreeView *list,
                                const OlLrcCandidate *candidates,
                                int count)
{
  ol_assert (list != NULL);
  GtkTreeIter iter;
  int i;
  ol_lrc_candidate_list_clear (list);
  GtkTreeStore *store = GTK_TREE_STORE (gtk_tree_view_get_model (list));
  for (i = 0; i < count; i++)
  {
    gtk_tree_store_append (store, &iter, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (store, &iter,
                        TITLE_COLUMN, candidates[i].title,
                        ARTIST_COLUMN, candidates[i].artist,
                        URL_COLUMN, candidates[i].url,
                        -1);
    /* ol_debugf ("  url: %s\n", candidates[i].url); */
    /* Select the first item */
    if (i == 0)
      gtk_tree_selection_select_iter (gtk_tree_view_get_selection (list),
                                      &iter);
  }
}

gboolean
ol_lrc_candidate_list_get_selected (GtkTreeView *list,
                                    OlLrcCandidate *candidate)
{
  ol_assert_ret (list != NULL, FALSE);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (list);
  if (selection != NULL)
  {
    GtkTreeIter iter;
    GtkTreeModel *model = NULL;
    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
      return FALSE;
    if (candidate != NULL)
    {
      char *title, *artist, *url;
      gtk_tree_model_get (model, &iter,
                          TITLE_COLUMN, &title,
                          ARTIST_COLUMN, &artist,
                          URL_COLUMN, &url,
                          -1);
      ol_lrc_candidate_set_title (candidate, title);
      ol_lrc_candidate_set_artist (candidate, artist);
      ol_lrc_candidate_set_url (candidate, url);
      g_free (title); g_free (artist); g_free (url);
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

void
ol_lrc_candidate_list_clear (GtkTreeView *list)
{
  GtkTreeStore *store = GTK_TREE_STORE (gtk_tree_view_get_model (list));
  gtk_tree_store_clear (store);
}
