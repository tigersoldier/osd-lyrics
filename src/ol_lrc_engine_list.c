#include "ol_lrc_engine_list.h"
#include "ol_intl.h"
#include "ol_utils.h"
#include "ol_debug.h"

void
ol_lrc_engine_list_init (GtkWidget *list)
{
  ol_assert (list != NULL);
  int i, nengine;
  char **download_engine = (char **)ol_lrc_fetch_get_engine_list (&nengine);
  for (i = 0; i < nengine; i++)
  {
    gtk_combo_box_append_text (GTK_COMBO_BOX (list),
                               _(download_engine[i]));
  }
}

OlLrcFetchEngine *
ol_lrc_engine_list_get_engine (GtkWidget *list)
{
  const char *name = ol_lrc_engine_list_get_name (list);
  return ol_lrc_fetch_get_engine (name);
}

const char *
ol_lrc_engine_list_get_name (GtkWidget *list)
{
  ol_assert_ret (list != NULL, NULL);
  ol_assert_ret (GTK_IS_COMBO_BOX (list), NULL);
  int index = gtk_combo_box_get_active (GTK_COMBO_BOX (list));
  int count = 0;
  const char **engine_list = ol_lrc_fetch_get_engine_list (&count);
  if (engine_list != NULL && index < count)
  {
    return engine_list[index];
  }
  return NULL;
}

void
ol_lrc_engine_list_set_name (GtkWidget *list, 
                             const char *name)
{
  ol_assert (list != NULL);
  ol_assert (GTK_IS_COMBO_BOX (list));
  GtkTreeModel *tree = gtk_combo_box_get_model (GTK_COMBO_BOX (list));
  GtkTreeIter iter;
  gboolean valid = gtk_tree_model_get_iter_first (tree, &iter);
  while (valid)
  {
    char *engine_name = NULL;
    gtk_tree_model_get (tree, &iter,
                        0, &engine_name,
                        -1);
    if (ol_stricmp (engine_name,
                    _(name),
                    strlen (engine_name)) == 0)
    {
      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (list),
                                     &iter);
      g_free (engine_name);
      return;
    }
    g_free (engine_name);
    valid = gtk_tree_model_iter_next (tree, &iter);
  }
}
