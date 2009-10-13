#include <stdlib.h>
#include "ol_lrc_fetch_ui.h"
#include "ol_lrc_fetch.h"
#include "ol_glade.h"
#include "ol_intl.h"

static GtkWidget *window = NULL;
static GtkTreeView *list = NULL;
static GtkTreeStore *store = NULL;
static GtkButton *download_button = NULL;
static OlLrcFetchEngine *engine = NULL;
static char *filepath = NULL;

enum {
  TITLE_COLUMN = 0,
  ARTIST_COLUMN,
  URL_COLUMN,
  COLUMN_COUNT,
};
static void ol_lrc_fetch_select_changed (GtkTreeSelection *selection, gpointer data);
static gboolean ol_lrc_fetch_ui_init ();
gboolean ol_lrc_fetch_cancel (GtkWidget *widget, gpointer data);

gboolean
ol_lrc_fetch_ui_cancel (GtkWidget *widget, gpointer data)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (window != NULL)
    gtk_widget_hide (window);
  exit (1);
}

gboolean
ol_lrc_fetch_ui_download (GtkWidget *widget, gpointer data)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  GtkTreeSelection *selection;
  OlLrcCandidate candidate;
  if (list != NULL)
    selection = gtk_tree_view_get_selection (list);
  if (selection != NULL)
  {
    GtkTreeIter iter;
    GtkTreeModel *model;
    gtk_tree_selection_get_selected (selection, &model, &iter);
    gtk_tree_model_get (model, &iter,
                        TITLE_COLUMN, &candidate.title,
                        ARTIST_COLUMN, &candidate.artist,
                        URL_COLUMN, &candidate.url,
                        -1);
    /* Get the index of selected candidate */
    /* GtkTreeIter first_iter; */
    /* gtk_tree_model_get_iter_first (model, &first_iter); */
    /* int index = 0; */
    /* gtk_tree_selection_iter_is_selected (selection, &iter); */
    /* TODO: download selected candidate */
    fprintf (stderr, "download pressed\n");
    engine->download (&candidate, filepath, "UTF-8");
    exit (0);
  }
}

static void
ol_lrc_fetch_select_changed (GtkTreeSelection *selection, gpointer data)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (download_button = NULL)
    gtk_widget_set_sensitive (GTK_WIDGET (download_button),
                              gtk_tree_selection_get_selected (selection, NULL, NULL));
}

static gboolean
ol_lrc_fetch_ui_init ()
{
  fprintf (stderr, "%s", __FUNCTION__);
  if (window == NULL)
  {
    window = ol_glade_get_widget ("downloaddialog");
    if (window == NULL)
      return FALSE;
    g_signal_connect (G_OBJECT (window),
                      "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete),
                      NULL);
  }
  if (store == NULL)
  {
    store = gtk_tree_store_new (COLUMN_COUNT,    /* Total number of columns */
                                G_TYPE_STRING,   /* Music title             */
                                G_TYPE_STRING,   /* Author                  */
                                G_TYPE_STRING);  /* URL                     */
  }
  if (list == NULL)
  {
    list = GTK_TREE_VIEW (ol_glade_get_widget ("candidate-list"));
    if (list == NULL)
      return FALSE;
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
    g_signal_connect (G_OBJECT (select), "changed",
                      G_CALLBACK (ol_lrc_fetch_select_changed),
                      NULL);
    
  }
  if (download_button == NULL)
  {
    download_button = GTK_BUTTON (ol_glade_get_widget ("lrc-download"));
  }
  return TRUE;
}

void
ol_lrc_fetch_ui_show (OlLrcFetchEngine *lrcengine,
                      const OlLrcCandidate *candidates,
                      int count,
                      const char *filename)
{
  fprintf (stderr, "%s\n", __FUNCTION__);
  if (window == NULL && !ol_lrc_fetch_ui_init ())
    return;
  if (lrcengine == NULL || candidates == NULL || count <= 0 || filename == NULL)
  {
    gtk_widget_hide (window);
    return;
  }
  if (filepath != NULL)
    g_free (filepath);
  filepath = g_strdup (filename);
  engine = lrcengine;
  GtkTreeIter iter;
  int i;
  gtk_tree_store_clear (store);
  for (i = 0; i < count; i++)
  {
    gtk_tree_store_append (store, &iter, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (store, &iter,
                        TITLE_COLUMN, candidates[i].title,
                        ARTIST_COLUMN, candidates[i].artist,
                        URL_COLUMN, candidates[i].url,
                        -1);
    /* Select the first item */
    if (i == 0)
      gtk_tree_selection_select_iter (gtk_tree_view_get_selection (list),
                                      &iter);
  }
  gtk_widget_show (window);
}
