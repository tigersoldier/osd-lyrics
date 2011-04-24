#include <stdlib.h>
#include "ol_lrc_fetch_ui.h"
#include "ol_lrc_fetch.h"
#include "ol_gui.h"
#include "ol_config.h"
#include "ol_lrc_fetch_module.h"
#include "ol_lrc_candidate_list.h"
#include "ol_debug.h"

static GtkWidget *window = NULL;
static GtkTreeView *list = NULL;
static GtkButton *download_button = NULL;
static OlLrcFetchEngine *engine = NULL;
static OlMusicInfo *info = NULL;
static char *filepath = NULL;

static void ol_lrc_fetch_select_changed (GtkTreeSelection *selection, gpointer data);
static gboolean internal_init ();
gboolean ol_lrc_fetch_cancel (GtkWidget *widget, gpointer data);

gboolean
ol_lrc_fetch_ui_cancel (GtkWidget *widget, gpointer data)
{
  ol_log_func ();
  if (window != NULL)
    gtk_widget_hide (window);
  return TRUE;
}

gboolean
ol_lrc_fetch_ui_download (GtkWidget *widget, gpointer data)
{
  ol_log_func ();
  OlLrcCandidate candidate = {{0}};
  if (ol_lrc_candidate_list_get_selected (list, &candidate))
  {
    ol_lrc_fetch_begin_download (engine, &candidate, info, filepath, NULL);
  }
  OlConfig *config = ol_config_get_instance ();
  GtkToggleButton *prompt_btn = GTK_TOGGLE_BUTTON (ol_gui_get_widget ("choose-do-not-prompt"));
  if (prompt_btn != NULL && config != NULL)
  {
    if (gtk_toggle_button_get_active (prompt_btn))
    {
      ol_config_set_bool (config, 
                          "Download", 
                          "download-first-lyric", 
                          TRUE);
    }
  }
  gtk_widget_hide (window);
  return TRUE;
}

static void
ol_lrc_fetch_select_changed (GtkTreeSelection *selection, gpointer data)
{
  ol_log_func ();
  if (download_button != NULL)
    gtk_widget_set_sensitive (GTK_WIDGET (download_button),
                              gtk_tree_selection_get_selected (selection, NULL, NULL));
}

static gboolean
internal_init ()
{
  ol_log_func ();
  if (window == NULL)
  {
    window = ol_gui_get_widget ("downloaddialog");
    if (window == NULL)
      return FALSE;
    g_signal_connect (G_OBJECT (window),
                      "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete),
                      NULL);
  }
  if (download_button == NULL)
  {
    download_button = GTK_BUTTON (ol_gui_get_widget ("lrc-download"));
  }
  if (list == NULL)
  {
    list = GTK_TREE_VIEW (ol_gui_get_widget ("candidate-list"));
    ol_lrc_candidate_list_init (list, 
                                G_CALLBACK (ol_lrc_fetch_select_changed));
  }
  return TRUE;
}

void
ol_lrc_fetch_ui_show (OlLrcFetchEngine *lrcengine,
                      const OlLrcCandidate *candidates,
                      int count,
                      const OlMusicInfo *music_info,
                      const char *filename)
{
  ol_log_func ();
  if (window == NULL && !internal_init ())
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
  ol_lrc_candidate_list_set_list (list, candidates, count);
  if (info == NULL)
    info = ol_music_info_new ();
  ol_music_info_copy (info, music_info);
    
  gboolean prompt = TRUE;
  OlConfig *config = ol_config_get_instance ();
  if (config != NULL)
    prompt = ol_config_get_bool (config, "Download", "download-first-lyric");
  if (prompt || count == 1)
    ol_lrc_fetch_ui_download (GTK_WIDGET (download_button), NULL);
  else
    gtk_widget_show (window);
}
