#include "ol_scroll_module.h"
#include "ol_music_info.h"
#include "ol_scroll_window.h"
#include "ol_lrc.h"

typedef struct _OlScrollModule OlScrollModule;

struct OlLrc;

struct _OlScrollModule
{
  OlMusicInfo music_info;
  gint duration;
  struct OlLrc *lrc;
  OlScrollWindow *scroll;
};

OlScrollModule* ol_scroll_module_new (struct OlDisplayModule *module);
void ol_scroll_module_free (struct OlDisplayModule *module);
void ol_scroll_module_set_music_info (struct OlDisplayModule *module, OlMusicInfo *music_info);
void ol_scroll_module_set_played_time (struct OlDisplayModule *module, int played_time);
void ol_scroll_module_set_lrc (struct OlDisplayModule *module, struct OlLrc *lrc_file);
void ol_scroll_module_set_duration (struct OlDisplayModule *module, int duration);
//void ol_scroll_module_set_music_info (struct OlDisplayModule *module, OlMusicInfo *music_info);
//void ol_scroll_module_set_player (struct OlDisplayModule *module, struct OlPlayer *player);
/*
void ol_osd_module_search_message (struct OlDisplayModule *module, const char *message);
void ol_osd_module_search_fail_message (struct OlDisplayModule *module, const char *message);
void ol_osd_module_download_fail_message (struct OlDisplayModule *module, const char *message);
void ol_osd_module_clear_message (struct OlDisplayModule *module);*/

static void
ol_scroll_module_init_scroll (OlScrollModule *module)
{
  ol_assert (module != NULL);
  module->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  g_object_ref_sink(module->scroll);
  if (module->scroll == NULL)
  {
    return;
  }
  gtk_window_set_opacity(GTK_WINDOW(module->scroll), 0.7); 
  gtk_widget_show(GTK_WIDGET (module->scroll));
}

OlScrollModule*
ol_scroll_module_new (struct OlDisplayModule *module)
{
  OlScrollModule *priv = g_new (OlScrollModule, 1);
  priv->scroll = NULL;
  priv->lrc = NULL;
  ol_music_info_init (&priv->music_info);
  ol_scroll_module_init_scroll (priv);
  return priv;
}

void
ol_scroll_module_free (struct OlDisplayModule *module)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (module == NULL)
    return;
  if (priv->scroll != NULL)
  {
    gtk_widget_destroy (GTK_WIDGET (priv->scroll));
    priv->scroll = NULL;
  }
  ol_music_info_clear (&priv->music_info);
  g_free (priv);
}

void
ol_scroll_module_set_music_info (struct OlDisplayModule *module, OlMusicInfo *music_info)
{
  ol_assert (music_info != NULL); 
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  ol_music_info_copy (&priv->music_info, music_info);
  /*
  if (priv->scroll != NULL)
  {
    ol_scroll_window_set_lyric (priv->scroll, NULL);
    }*/
}

void
ol_scroll_module_set_played_time (struct OlDisplayModule *module, int played_time)
{
  ol_assert (module != NULL);
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->lrc != NULL && priv->scroll != NULL)
  {
    double percentage;
    int lyric_id;
    ol_lrc_get_lyric_by_time (priv->lrc,
                              played_time,
                              priv->duration,
                              NULL,
                              &percentage,
                              &lyric_id);
    const struct OlLrcItem *info = ol_lrc_get_item (priv->lrc,lyric_id);
    if (lyric_id == -1)
    {
      ol_scroll_window_set_lyric (priv->scroll, -1);
      return;
    }
    else
    {
      /*change to the next lyric line*/
      if (lyric_id != ol_scroll_window_get_current_lyric_id (priv->scroll))
      {
	ol_debugf ("lyris:%s\n",ol_lrc_item_get_lyric(info));
        ol_scroll_window_set_lyric (priv->scroll, ol_lrc_item_get_id (info));
      }
      /*set the percentage of the current lyric line*/
      else
        ol_scroll_window_set_current_percentage (priv->scroll, percentage);
    }
    
  }
  else {
    ol_scroll_window_set_lyric (priv->scroll, -1);
  }
}



void
ol_scroll_module_set_lrc (struct OlDisplayModule *module, struct OlLrc *lrc_file)
{
  ol_log_func ();
  ol_assert (module != NULL);
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  priv->lrc = lrc_file;
  if(priv->scroll == NULL)
  {
      priv->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  }
  if (lrc_file == NULL)
    ol_scroll_window_set_whole_lyrics(priv->scroll, NULL, 0);
  else {
    /*dump the whole lyrics of a song*/
    int count = ol_lrc_item_count (lrc_file);
    GPtrArray *whole_lyrics = g_ptr_array_new_with_free_func (g_free);
    const struct OlLrcItem *info = NULL;
    int i;
    for (i = 0; i < count; i++) {
      info = ol_lrc_get_item (lrc_file, i);
      g_ptr_array_add (whole_lyrics, g_strdup (ol_lrc_item_get_lyric (info)));
    }
    ol_scroll_window_set_whole_lyrics(priv->scroll, whole_lyrics, count);
    g_ptr_array_unref (whole_lyrics);
  }
}

void
ol_scroll_module_set_duration (struct OlDisplayModule *module, int duration)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (module == NULL)
    return;
  priv->duration = duration;
}

struct OlDisplayClass*
ol_scroll_module_get_class ()
{
  struct OlDisplayClass *klass = ol_display_class_new ("scroll",
                                                       (OlDisplayInitFunc) ol_scroll_module_new,
                                                       ol_scroll_module_free);
  /* klass->clear_message = ol_scroll_module_clear_message; */
  /* klass->download_fail_message = ol_scroll_module_download_fail_message; */
  /* klass->search_fail_message = ol_scroll_module_search_fail_message; */
  /* klass->search_message = ol_scroll_module_search_message; */
  klass->set_duration = ol_scroll_module_set_duration;
  klass->set_lrc = ol_scroll_module_set_lrc;
  /* klass->set_message = ol_scroll_module_set_message; */
  klass->set_music_info = ol_scroll_module_set_music_info;
  klass->set_played_time = ol_scroll_module_set_played_time;
  /* klass->set_player = ol_scroll_module_set_player; */
  /* klass->set_status = ol_scroll_module_set_status; */
  return klass;
}
